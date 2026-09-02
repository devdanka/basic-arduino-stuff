
#include <Arduino.h>

int pinRelay = 6;
int pinPhotoresistor = A0;
int pinPhotoPower = 5;

int threshold = 200;
int margin = 20;

bool relayState = false;
int photoresistorValue;
int wait = 100;

void setup() {
  pinMode(pinPhotoPower, OUTPUT);
  digitalWrite(pinPhotoPower, HIGH);
  pinMode(pinRelay, OUTPUT);
  digitalWrite(pinRelay, HIGH);
  Serial.begin(9600);
}

void loop() {
  photoresistorValue = analogRead(pinPhotoresistor);

  if (!relayState && photoresistorValue < threshold - margin) {
    relayState = true;
    digitalWrite(pinRelay, LOW);
  } else if (relayState && photoresistorValue > threshold + margin) {
    relayState = false;
    digitalWrite(pinRelay, HIGH);
  }

  if (relayState) {
    Serial.println(String(photoresistorValue) + " relay on");
  } else {
    Serial.println(String(photoresistorValue) + " relay off");
  }

  delay(wait);
}
