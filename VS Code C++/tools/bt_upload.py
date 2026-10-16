"""Upload ESP32 firmware over Bluetooth, from VS Code.

platformio.ini runs this as the custom upload command for the esp32-bt env, so
`pio run -e esp32-bt -t upload` sends the binary over the Bluetooth link
instead of the USB cable.

This does not talk to the ESP32 bootloader - it cannot, the bootloader only
speaks UART and it runs instead of your sketch, so Bluetooth does not exist
while it is in charge. Instead the *running sketch* receives the image and
writes it to the spare OTA slot itself. The receiving half is
receiveFirmwareOverBluetooth() in src/ESP32/000-blinkingLED/000-blinkingLED.cpp,
and the protocol is documented above it.

That means the board must already be running firmware that contains the
receiver. Send it something that does not and it goes deaf - reflash over the
cable with `pio run -e esp32 -t upload` and you are back.

Usage:
    python tools/bt_upload.py firmware.bin
    python tools/bt_upload.py firmware.bin --port COM7 --password YOUR_OTA_PASSWORD
"""

import argparse
import re
import sys
import time

try:
    import serial
    from serial.tools import list_ports
except ImportError:
    sys.exit("pyserial is missing. Install it with: pio pkg exec -- pip install pyserial")

CHUNK = 1024
LINK_SETTLE = 2.5          # Windows dials the RFCOMM link after the port opens
HELLO_TIMEOUT = 24.0       # total time to keep asking one port for a READY
RETRY_EVERY = 3.0          # resend the header this often while waiting
CHUNK_TIMEOUT = 20.0       # a flash-write ack should never take this long


def candidate_ports():
    """Bluetooth COM ports, most likely one first.

    Windows makes two ports per pairing: an outgoing one that dials the device
    and an incoming one that only sits and listens. Only the outgoing one is
    any use, and writing to the other one blocks until it times out. They are
    told apart by the device address embedded in the hwid - the listen-only
    port's is all zeros, the real one carries the board's Bluetooth MAC.
    """
    outgoing, listening = [], []
    for p in list_ports.comports():
        hwid = p.hwid or ""
        text = "{} {}".format(p.description or "", hwid).lower()
        if "bluetooth" not in text and "bthenum" not in text:
            continue
        found = re.search(r"&0&([0-9a-fA-F]{12})_", hwid)
        if found and found.group(1) != "0" * 12:
            outgoing.append(p.device)
        else:
            listening.append(p.device)
    return outgoing + listening


def handshake(port, size, password, verbose=True):
    """Open one port and try to get a READY out of it. Returns the open port."""
    if verbose:
        print("Trying {}...".format(port), end=" ", flush=True)
    try:
        # The baud rate is meaningless over Bluetooth SPP - the link negotiates
        # its own speed - but pyserial insists on a number. The short write
        # timeout is what stops the listen-only port hanging us for 30 s.
        link = serial.Serial(port, 115200, timeout=0.5, write_timeout=5)
    except (OSError, serial.SerialException) as exc:
        if verbose:
            print("cannot open ({})".format(getattr(exc, "strerror", None) or exc))
        return None

    # Opening the COM port returns at once, but Windows only dials the actual
    # Bluetooth link afterwards, and anything written before that link is up
    # gets dropped on the floor. Hence the settle, and hence re-sending the
    # header rather than asking once and giving up.
    time.sleep(LINK_SETTLE)
    try:
        link.reset_input_buffer()
    except (OSError, serial.SerialException):
        pass

    header = "BTOTA {} {}\n".format(size, password).encode()
    deadline = time.time() + HELLO_TIMEOUT
    buf = b""

    while time.time() < deadline:
        try:
            link.write(header)
            link.flush()
        except (serial.SerialTimeoutException, OSError, serial.SerialException):
            link.close()
            if verbose:
                print("not writable - this is the listen-only port.")
            return None

        until = time.time() + RETRY_EVERY
        while time.time() < until:
            try:
                buf += link.read(64)
            except (OSError, serial.SerialException):
                link.close()
                if verbose:
                    print("link dropped.")
                return None

            while b"\n" in buf:
                reply, _, buf = buf.partition(b"\n")
                reply = reply.strip().decode("utf-8", "replace")
                if reply == "READY":
                    if verbose:
                        print("ready.")
                    link.write_timeout = 30     # the data phase needs longer
                    return link
                if reply.startswith("ERR"):
                    link.close()
                    sys.exit("Board refused the upload: {}".format(reply[4:].strip()))
                # anything else is ordinary log output from the board's
                # Bluetooth monitor - ignore it and keep listening
        if verbose:
            print(".", end="", flush=True)

    link.close()
    if verbose:
        print(" no answer.")
    return None


def send(link, image):
    total = len(image)
    sent = 0
    started = time.time()

    while sent < total:
        block = image[sent:sent + CHUNK]
        link.write(block)
        link.flush()

        # One ack per chunk. This is flow control, not politeness: flash writes
        # are slower than the link, and without waiting the board's receive
        # queue overruns and the image quietly ends up corrupt.
        deadline = time.time() + CHUNK_TIMEOUT
        ack = b""
        while not ack and time.time() < deadline:
            ack = link.read(1)
        if ack != b"K":
            raise IOError(
                "no ack after {} of {} bytes - link dropped or board reset".format(sent, total)
            )

        sent += len(block)
        pct = sent * 100 // total
        rate = sent / max(time.time() - started, 0.001) / 1024
        print("\r  {:3d}%  {:,} / {:,} bytes  {:.1f} kB/s".format(pct, sent, total, rate),
              end="", flush=True)

    print()
    return time.time() - started


def main():
    ap = argparse.ArgumentParser(description="Send ESP32 firmware over Bluetooth.")
    ap.add_argument("firmware", help="path to firmware.bin")
    ap.add_argument("--port", help="Bluetooth COM port; auto-detected if omitted")
    ap.add_argument("--password", default="YOUR_OTA_PASSWORD",
                    help="must match OTA_PASSWORD in the sketch's secrets.h")
    args = ap.parse_args()

    with open(args.firmware, "rb") as fh:
        image = fh.read()
    if not image:
        sys.exit("{} is empty.".format(args.firmware))

    print("Firmware: {:,} bytes".format(len(image)))

    ports = [args.port] if args.port else candidate_ports()
    if not ports:
        sys.exit(
            "No Bluetooth COM port found.\n"
            "Pair the board first: Settings > Bluetooth & devices > Add device >\n"
            "Bluetooth > ESP32-Monitor. Then pass --port COMn if it still is not found."
        )

    link = None
    for port in ports:
        link = handshake(port, len(image), args.password)
        if link:
            break

    if not link:
        sys.exit(
            "No board answered on: {}\n"
            "Is it powered, paired, and running firmware that contains the receiver?\n"
            "If you flashed something without it, reflash over USB:\n"
            "  pio run -e esp32 -t upload".format(", ".join(ports))
        )

    try:
        elapsed = send(link, image)
        link.timeout = 30
        tail = link.readline().strip().decode("utf-8", "replace")
        if tail != "OK":
            sys.exit("Board did not confirm: {}".format(tail or "(silence)"))
        print("Done in {:.1f} s. Board is rebooting into the new firmware.".format(elapsed))
    except (IOError, serial.SerialException) as exc:
        sys.exit("Upload failed: {}".format(exc))
    finally:
        link.close()


if __name__ == "__main__":
    main()
