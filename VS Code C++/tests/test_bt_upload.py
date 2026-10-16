import contextlib
import io
import os
from pathlib import Path
import sys
import types
import unittest
from unittest.mock import patch


class SerialException(Exception):
    pass


class SerialTimeoutException(SerialException):
    pass


serial_module = types.ModuleType("serial")
serial_module.SerialException = SerialException
serial_module.SerialTimeoutException = SerialTimeoutException
serial_module.Serial = None
serial_tools = types.ModuleType("serial.tools")
serial_tools.list_ports = types.SimpleNamespace(comports=lambda: [])
source = Path(os.environ.get("BT_UPLOAD_SCRIPT", Path(__file__).resolve().parents[1] / "tools" / "bt_upload.py"))
uploader = types.ModuleType("bt_upload_under_test")
with patch.dict(sys.modules, {"serial": serial_module, "serial.tools": serial_tools}):
    exec(compile(source.read_bytes(), str(source), "exec"), uploader.__dict__)


class Clock:
    def __init__(self):
        self.now = 0.0

    def monotonic(self):
        return self.now

    def time(self):
        return 1700000000.0 + self.now

    def sleep(self, seconds):
        self.now += seconds


class BoardLink:
    def __init__(self, clock, ready_at=0.0, response_chunks=None, error=None, partial=False):
        self.clock = clock
        self.ready_at = ready_at
        self.response_chunks = response_chunks
        self.error = error
        self.partial = partial
        self.timeout = 0.5
        self.write_timeout = 5
        self.closed = False
        self.writes = []
        self.pending = bytearray()
        self.replies = []
        self.image = bytearray()
        self.expected_size = None
        self.acknowledged = 0
        self.header_count = 0

    def reset_input_buffer(self):
        self.replies.clear()

    def close(self):
        self.closed = True

    def write(self, data):
        if self.error == "write":
            raise SerialTimeoutException("test write failure")
        self.writes.append(bytes(data))
        count = len(data) // 2 if self.partial else len(data)
        self.pending.extend(data[:count])
        self.process()
        return count

    def flush(self):
        if self.error == "flush":
            raise SerialException("test flush failure")

    def process(self):
        if self.clock.now < self.ready_at:
            return
        if self.expected_size is None:
            if b"\n" not in self.pending:
                return
            header, _, remainder = self.pending.partition(b"\n")
            self.pending = bytearray(remainder)
            fields = header.split()
            if len(fields) != 3 or fields[0] != b"BTOTA":
                self.replies.append(b"ERR malformed header\n")
                return
            self.header_count += 1
            self.expected_size = int(fields[1])
            self.replies.extend(self.response_chunks or [b"READY\n"])
        if self.pending:
            self.image.extend(self.pending[:self.expected_size - len(self.image)])
            self.pending.clear()
        complete_chunks = min(len(self.image), self.expected_size) // uploader.CHUNK
        if len(self.image) >= self.expected_size and self.expected_size % uploader.CHUNK:
            complete_chunks += 1
        while self.acknowledged < complete_chunks:
            self.replies.append(b"K")
            self.acknowledged += 1

    def read(self, size):
        if self.error == "read":
            raise SerialException("test read failure")
        self.process()
        if not self.replies:
            self.clock.sleep(self.timeout)
            self.process()
        if not self.replies:
            return b""
        result = self.replies[0][:size]
        self.replies[0] = self.replies[0][size:]
        if not self.replies[0]:
            self.replies.pop(0)
        return result


class BluetoothUploadTests(unittest.TestCase):
    def setUp(self):
        self.clock = Clock()
        self.image = bytes(range(256)) * 9 + b"firmware-tail"
        self.password = "test-only-password"
        self.output = io.StringIO()
        self.enterContext(contextlib.redirect_stdout(self.output))
        self.enterContext(patch.object(uploader, "time", self.clock))

    def connect(self, **kwargs):
        self.link = BoardLink(self.clock, **kwargs)
        self.enterContext(patch.object(uploader.serial, "Serial", return_value=self.link))
        return self.link

    def handshake(self):
        return uploader.handshake("TEST", len(self.image), self.password, verbose=False)

    def assert_one_header(self):
        expected = "BTOTA {} {}\n".format(len(self.image), self.password).encode()
        self.assertEqual([part for part in self.link.writes if part.startswith(b"BTOTA ")], [expected])

    def test_normal_handshake_and_multichunk_image(self):
        link = self.connect()
        self.assertIs(self.handshake(), link)
        uploader.send(link, self.image)
        self.assert_one_header()
        self.assertEqual(link.image, self.image)
        self.assertEqual(link.write_timeout, 30)

    def test_busy_wifi_for_twenty_seconds_does_not_corrupt_image(self):
        link = self.connect(ready_at=20.0)
        self.assertIs(self.handshake(), link)
        uploader.send(link, self.image)
        self.assertEqual(link.image, self.image)
        self.assert_one_header()

    def test_split_ready_and_monitor_logs(self):
        link = self.connect(response_chunks=[b"WiFi reconnecting\r\nR", b"EA", b"DY\r\n"])
        self.assertIs(self.handshake(), link)
        uploader.send(link, self.image)
        self.assert_one_header()
        self.assertEqual(link.image, self.image)

    def test_auth_rejection_closes_without_payload(self):
        link = self.connect(response_chunks=[b"ERR wrong password\r\n"])
        with self.assertRaisesRegex(SystemExit, "Board refused the upload: wrong password"):
            self.handshake()
        self.assertTrue(link.closed)
        self.assert_one_header()
        self.assertEqual(link.image, b"")

    def test_missing_ready_closes_without_repeating_header(self):
        link = self.connect(ready_at=float("inf"))
        self.assertIsNone(self.handshake())
        self.assertTrue(link.closed)
        self.assert_one_header()
        self.assertLessEqual(self.clock.now, uploader.LINK_SETTLE + uploader.HELLO_TIMEOUT + 0.5)

    def test_open_failure(self):
        self.enterContext(patch.object(uploader.serial, "Serial", side_effect=SerialException("test open failure")))
        self.assertIsNone(self.handshake())

    def test_write_failure_closes(self):
        link = self.connect(error="write")
        self.assertIsNone(self.handshake())
        self.assertTrue(link.closed)

    def test_flush_failure_closes(self):
        link = self.connect(error="flush")
        self.assertIsNone(self.handshake())
        self.assertTrue(link.closed)

    def test_read_failure_closes(self):
        link = self.connect(error="read")
        self.assertIsNone(self.handshake())
        self.assertTrue(link.closed)
        self.assert_one_header()

    def test_partial_header_write_closes_without_resending(self):
        link = self.connect(partial=True)
        self.assertIsNone(self.handshake())
        self.assertTrue(link.closed)
        self.assert_one_header()

    def test_handshake_uses_monotonic_clock(self):
        link = self.connect(ready_at=20.0)
        with patch.object(self.clock, "time", side_effect=AssertionError("handshake must not use wall clock")):
            self.assertIs(self.handshake(), link)
        self.assert_one_header()


if __name__ == "__main__":
    unittest.main(verbosity=2)
