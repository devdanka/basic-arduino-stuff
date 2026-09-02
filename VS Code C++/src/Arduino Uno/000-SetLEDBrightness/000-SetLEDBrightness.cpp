#include <Arduino.h>

int readPinUp = A0;
int readPinDown = A2;
int redPin = 3;
int readUp;
int readDown;
int brightness = 0;
int wait = 1;

void setup() {
  pinMode(readPinUp, INPUT);
  pinMode(readPinDown, INPUT);
  pinMode(readPinDown, INPUT);
  pinMode(redPin, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  readUp = digitalRead(readPinUp);
  readDown = digitalRead(readPinDown);
  Serial.println(brightness);
  if (readUp == 0) {
    brightness = brightness + 1;
    if (brightness >= 254) {
      brightness = 255;
    }
  }
  
  if (readDown == 0) {
    brightness = brightness - 1;
    if (brightness <= 1) {
      brightness = 0;
    }
  }

  analogWrite(redPin, brightness);
  delay(wait);
}