#include <Arduino.h>
#include <WirelessUpload.h>

#include "secrets.h"

int ledPin = 2;
unsigned long blinkInterval = 1000;

unsigned long lastToggle = 0;
bool ledOn = false;

void setup() {
  pinMode(ledPin, OUTPUT);
  Serial.begin(115200);
  delay(1000);

  WirelessUpload.setStaticIP(IPAddress(192, 168, 1, 184),
                             IPAddress(192, 168, 1, 1),
                             IPAddress(255, 255, 255, 0),
                             IPAddress(192, 168, 1, 1));
  WirelessUpload.begin(WIFI_SSID, WIFI_PASSWORD, "esp32", OTA_PASSWORD);
}

void loop() {
  WirelessUpload.handle();

  if (millis() - lastToggle >= blinkInterval) {
    lastToggle = millis();
    ledOn = !ledOn;
    digitalWrite(ledPin, ledOn);
  }
}
