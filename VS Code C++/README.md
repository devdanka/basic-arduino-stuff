# PlatformIO Projects

15 Arduino Uno and 6 ESP32 examples, each with a short README.

- [Arduino Uno examples](src/Basics%20of%20Arduino%20Uno)
- [ESP32 examples](src/Basics%20of%20ESP32)

Run commands from this folder. Build with `pio run -e <environment>`; add `-t upload` to upload. Environment names are listed in `platformio.ini`.

For Wi-Fi sketches, copy the nearby `secrets.example.h` to `secrets.h` and fill in your credentials. Replace `YOUR_OTA_PASSWORD` consistently in the firmware and upload settings. Adjust the network addresses for your network.

The web LED example also needs `pio run -e esp32 -t uploadfs`. Bluetooth uploads use `tools/bt_upload.py` and require `pyserial`.
