#include <Arduino.h>
#include <WirelessUpload.h>

int ledPin = 2;
unsigned long blinkInterval = 500;

unsigned long lastToggle = 0;
bool ledOn = false;

void setup() {
  pinMode(ledPin, OUTPUT);
  Serial.begin(115200);
  delay(1000);

  WirelessUpload.begin(nullptr, nullptr, nullptr, "YOUR_OTA_PASSWORD");
}

void loop() {
  WirelessUpload.handle();

  if (millis() - lastToggle >= blinkInterval) {
    lastToggle = millis();
    ledOn = !ledOn;
    digitalWrite(ledPin, ledOn);
  }
}