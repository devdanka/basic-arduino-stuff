# Wireless Upload Library

Lets ESP32 sketches receive firmware updates over Wi-Fi or Bluetooth. It also supports Bluetooth-only builds.

Call `WirelessUpload.begin()` in `setup()` and `WirelessUpload.handle()` regularly in `loop()` so the board can receive updates.
