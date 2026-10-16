// ---------------------------------------------------------------------------
// WirelessUpload - keep any sketch reflashable without the cable.
//
// The catch with wireless uploading is that it is not a mode the board sits
// in. It is code that has to be present in whatever firmware is running, so
// the moment you upload a sketch that lacks it, the board goes deaf and the
// only way back is the USB cable. This library exists so that keeping it is
// two lines instead of a hundred.
//
//     #include <WirelessUpload.h>
//     #include "secrets.h"
//
//     void setup() {
//       WirelessUpload.begin(WIFI_SSID, WIFI_PASSWORD, "esp32", OTA_PASSWORD);
//     }
//
//     void loop() {
//       WirelessUpload.handle();     // must run often - see the warning below
//       ...your code...
//     }
//
// Put those in and the sketch can be replaced two ways:
//     pio run -e <env>-air -t upload    ; over WiFi,      ~15 s
//     pio run -e <env>-bt  -t upload    ; over Bluetooth, ~26 kB/s
//
// Pass a null ssid to skip WiFi entirely and run Bluetooth-only, which is
// useful for a board somewhere with no network to join.
//
// WARNING: handle() has to be called often - many times a second. A delay()
// in your loop starves it and uploads will time out partway. Blink with
// millis() instead of delay(); there is an example in the README.
// ---------------------------------------------------------------------------

#pragma once

#include <Arduino.h>
#include <IPAddress.h>

class BluetoothSerial;

class WirelessUploadClass {
 public:
  // Optional, and must be called before begin(). Without it the router
  // decides the address, which means reading it off the log after every boot.
  void setStaticIP(IPAddress ip, IPAddress gateway, IPAddress subnet, IPAddress dns);

  // Brings up Bluetooth, then WiFi and the OTA listener. Returns true if WiFi
  // joined; Bluetooth uploading still works when it returns false, which is
  // most of the point. Pass ssid = nullptr for Bluetooth only.
  bool begin(const char* ssid,
             const char* wifiPassword,
             const char* hostname,
             const char* otaPassword,
             const char* btName = "ESP32-Monitor");

  // Call this constantly from loop(). It services both upload paths and
  // reconnects WiFi if it drops.
  void handle();

  // Writes to the USB cable and the Bluetooth link at once, so the same
  // messages reach you whichever you are listening on. Use it exactly like
  // Serial: WirelessUpload.log().println("hello").
  Print& log();

  bool wifiConnected() const;
  bool bluetoothReady() const { return _btReady; }
  bool uploading() const { return _uploading; }

 private:
  bool connectWiFi();
  void startOTA();
  void pumpBluetooth();
  void receiveOverBluetooth(const String& header);

  const char* _ssid = nullptr;
  const char* _wifiPassword = nullptr;
  const char* _hostname = "esp32";
  const char* _otaPassword = "";

  bool _useStaticIP = false;
  IPAddress _ip, _gateway, _subnet, _dns;

  bool _btReady = false;
  bool _uploading = false;
  bool _otaStarted = false;

  String _btLine;
  unsigned long _lastCheck = 0;
};

extern WirelessUploadClass WirelessUpload;
