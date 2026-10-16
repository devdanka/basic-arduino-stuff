#include "WirelessUpload.h"

// Build with -DWIRELESS_UPLOAD_BT_ONLY to leave WiFi out entirely. Bluetooth
// uploading does not need it, and dropping the WiFi stack and ArduinoOTA saves
// a large chunk of flash - which is also a proportionally faster upload, since
// every byte has to cross a ~26 kB/s link. In that mode the sketch needs no
// SSID, no password, and no secrets.h at all.
#ifndef WIRELESS_UPLOAD_BT_ONLY
#include <WiFi.h>
#include <ArduinoOTA.h>
#endif

#include <BluetoothSerial.h>
#include <Update.h>

namespace {

BluetoothSerial SerialBT;
bool* btReadyFlag = nullptr;

// Fans every log line out to both the cable and the Bluetooth link, so the
// same messages reach you whichever one you happen to be listening on. Print
// supplies println() and printf() for free once write() exists.
class MirrorPrint : public Print {
 public:
  size_t write(uint8_t c) override {
    Serial.write(c);
    if (btReadyFlag && *btReadyFlag) SerialBT.write(c);
    return 1;
  }
  size_t write(const uint8_t* buf, size_t size) override {
    Serial.write(buf, size);
    if (btReadyFlag && *btReadyFlag) SerialBT.write(buf, size);
    return size;
  }
};

MirrorPrint mirror;

const unsigned long kWiFiTimeout = 20000;   // give up joining after 20 s
const unsigned long kRecheckEvery = 10000;  // how often to notice a drop

}  // namespace

WirelessUploadClass WirelessUpload;

Print& WirelessUploadClass::log() { return mirror; }

bool WirelessUploadClass::wifiConnected() const {
#ifdef WIRELESS_UPLOAD_BT_ONLY
  return false;
#else
  return WiFi.status() == WL_CONNECTED;
#endif
}

void WirelessUploadClass::setStaticIP(IPAddress ip, IPAddress gateway,
                                      IPAddress subnet, IPAddress dns) {
  _useStaticIP = true;
  _ip = ip;
  _gateway = gateway;
  _subnet = subnet;
  _dns = dns;
}

bool WirelessUploadClass::begin(const char* ssid, const char* wifiPassword,
                                const char* hostname, const char* otaPassword,
                                const char* btName) {
  _ssid = ssid;
  _wifiPassword = wifiPassword;
  _hostname = hostname ? hostname : "esp32";
  _otaPassword = otaPassword ? otaPassword : "";

  btReadyFlag = &_btReady;

  // Bluetooth first, so even the WiFi failure messages below are mirrored to
  // it. This is the path that still works when there is no network at all.
  _btReady = SerialBT.begin(btName);
  mirror.printf(_btReady ? "Bluetooth upload ready as \"%s\".\n"
                         : "Bluetooth failed to start (\"%s\").\n", btName);

#ifdef WIRELESS_UPLOAD_BT_ONLY
  mirror.println("Bluetooth only - this build has no WiFi in it.");
  return false;
#else
  if (!_ssid) {
    mirror.println("No WiFi configured - Bluetooth uploads only.");
    return false;
  }

  if (!connectWiFi()) {
    return false;
  }
  startOTA();
  return true;
#endif
}

#ifndef WIRELESS_UPLOAD_BT_ONLY
bool WirelessUploadClass::connectWiFi() {
  // Without this you just get a silent 20 s timeout and no hint as to why.
  if (String(_ssid) == "YOUR_WIFI_NAME") {
    mirror.println("secrets.h still has the placeholder network. Fill it in.");
    return false;
  }

  mirror.print("Connecting to ");
  mirror.println(_ssid);

  WiFi.mode(WIFI_STA);          // join a network, don't become one

  // Must come before begin(). If it fails the board still joins, on DHCP.
  if (_useStaticIP && !WiFi.config(_ip, _gateway, _subnet, _dns)) {
    mirror.println("Static IP rejected, falling back to DHCP.");
  }

  WiFi.begin(_ssid, _wifiPassword);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < kWiFiTimeout) {
    pumpBluetooth();
    delay(10);
  }

  if (WiFi.status() != WL_CONNECTED) {
    mirror.println("No WiFi. Check the name, the password, and that it is 2.4 GHz.");
    mirror.println("Bluetooth uploads still work.");
    return false;
  }

  mirror.print("WiFi OTA ready at ");
  mirror.println(WiFi.localIP());
  return true;
}

void WirelessUploadClass::startOTA() {
  ArduinoOTA.setHostname(_hostname);

  // Without a password anyone on the network can overwrite the firmware.
  if (strlen(_otaPassword) > 0) {
    ArduinoOTA.setPassword(_otaPassword);
  }

  ArduinoOTA.onStart([]() {
    WirelessUpload._uploading = true;
    // U_FLASH is the program; the other command is the filesystem image,
    // which arrives when you run -t uploadfs instead of -t upload.
    mirror.println(ArduinoOTA.getCommand() == U_FLASH
                       ? "OTA starting: firmware"
                       : "OTA starting: filesystem");
  });

  ArduinoOTA.onProgress([](unsigned int done, unsigned int total) {
    mirror.printf("  %u%%\r", total ? (done * 100U) / total : 0);
  });

  ArduinoOTA.onEnd([]() {
    WirelessUpload._uploading = false;
    mirror.println("\nOTA done, rebooting.");
  });

  ArduinoOTA.onError([](ota_error_t error) {
    WirelessUpload._uploading = false;
    mirror.printf("\nOTA failed (%u): ", error);
    switch (error) {
      case OTA_AUTH_ERROR:    mirror.println("wrong password"); break;
      case OTA_BEGIN_ERROR:   mirror.println("no room in the spare OTA slot"); break;
      case OTA_CONNECT_ERROR: mirror.println("could not reach the board"); break;
      case OTA_RECEIVE_ERROR: mirror.println("transfer cut off partway"); break;
      case OTA_END_ERROR:     mirror.println("flash write did not verify"); break;
      default:                mirror.println("unknown"); break;
    }
  });

  // Also starts mDNS and advertises _arduino._tcp on port 3232, which is what
  // puts the board under "Network ports" in the Arduino IDE.
  ArduinoOTA.begin();
  _otaStarted = true;
}
#else
// BT-only build: these never run, but the class still declares them.
bool WirelessUploadClass::connectWiFi() { return false; }
void WirelessUploadClass::startOTA() {}
#endif  // WIRELESS_UPLOAD_BT_ONLY

// ---------------------------------------------------------------------------
// Uploading over Bluetooth.
//
// The ROM bootloader cannot do this - it only speaks UART, and it runs instead
// of the sketch, so Bluetooth does not exist while it is in charge. So we skip
// the bootloader: this running sketch takes the image in and writes it to the
// spare OTA slot itself, with the same Update library ArduinoOTA drives over
// WiFi. Only the pipe differs.
//
// The other half is tools/bt_upload.py. Protocol, in full:
//
//   host -> "BTOTA <size> <password>\n"
//   esp  -> "READY\n"          (or "ERR <reason>\n" and we are done)
//   host -> <size> bytes, in 1024-byte chunks
//   esp  -> "K" after each chunk it has written
//   esp  -> "OK\n", then reboots into the new firmware
//
// The per-chunk ack is flow control, not politeness. Flash writes are slower
// than the link, and without it the sender overruns the receive queue and the
// image silently ends up corrupt.
// ---------------------------------------------------------------------------
void WirelessUploadClass::receiveOverBluetooth(const String& header) {
  int sp1 = header.indexOf(' ');
  int sp2 = header.indexOf(' ', sp1 + 1);
  if (sp1 < 0 || sp2 < 0) {
    SerialBT.println("ERR malformed header");
    return;
  }

  size_t size = (size_t)header.substring(sp1 + 1, sp2).toInt();
  String pass = header.substring(sp2 + 1);
  pass.trim();

  // Anything paired with the board could otherwise push its own firmware.
  if (pass != String(_otaPassword)) {
    SerialBT.println("ERR wrong password");
    return;
  }
  if (size == 0 || !Update.begin(size)) {
    SerialBT.printf("ERR cannot begin (%u bytes, error %u)\n", size, Update.getError());
    return;
  }

  _uploading = true;
  mirror.printf("Bluetooth upload starting, %u bytes\n", size);
  SerialBT.println("READY");

  const size_t CHUNK = 1024;
  uint8_t buf[CHUNK];
  size_t got = 0;
  unsigned long lastByte = millis();

  while (got < size) {
    size_t want = (size - got < CHUNK) ? (size - got) : CHUNK;
    size_t have = 0;
    while (have < want) {
      if (SerialBT.available()) {
        buf[have++] = (uint8_t)SerialBT.read();
        lastByte = millis();
      } else if (millis() - lastByte > 15000) {
        Update.abort();
        SerialBT.println("ERR timed out waiting for data");
        mirror.println("Bluetooth upload timed out.");
        _uploading = false;
        return;
      }
    }

    if (Update.write(buf, want) != want) {
      Update.abort();
      SerialBT.println("ERR flash write failed");
      mirror.println("Bluetooth upload failed on write.");
      _uploading = false;
      return;
    }

    got += want;
    SerialBT.write('K');         // "that chunk is flashed, send the next"
  }

  if (!Update.end(true)) {
    SerialBT.printf("ERR did not verify (%u)\n", Update.getError());
    mirror.println("Bluetooth upload did not verify.");
    _uploading = false;
    return;
  }

  SerialBT.println("OK");
  SerialBT.flush();
  mirror.println("Bluetooth upload done, rebooting.");
  delay(300);                    // let the last bytes drain before the reset
  ESP.restart();
}

// Watches the Bluetooth link for the one command we care about. Anything else
// arriving is ignored, so the link stays usable as a plain serial monitor.
void WirelessUploadClass::pumpBluetooth() {
  if (!_btReady) {
    return;
  }
  while (SerialBT.available()) {
    char c = (char)SerialBT.read();
    if (c == '\n') {
      String line = _btLine;
      _btLine = "";
      line.trim();
      if (line.startsWith("BTOTA ")) {
        receiveOverBluetooth(line);
      }
      return;
    }
    if (_btLine.length() < 120) {   // ignore anything absurd rather than grow
      _btLine += c;
    }
  }
}

void WirelessUploadClass::handle() {
#ifndef WIRELESS_UPLOAD_BT_ONLY
  if (_otaStarted) {
    ArduinoOTA.handle();
  }
#endif

  pumpBluetooth();

#ifndef WIRELESS_UPLOAD_BT_ONLY
  if (!_ssid || millis() - _lastCheck < kRecheckEvery) {
    return;
  }
  _lastCheck = millis();

  if (WiFi.status() == WL_CONNECTED) {
    if (!_otaStarted) {
      startOTA();
    }
  } else {
    _otaStarted = false;
    mirror.println("WiFi dropped, reconnecting...");
    if (connectWiFi()) {
      startOTA();
    }
  }
#endif
}
