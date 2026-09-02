#include <Arduino.h>

int latchPin=11;
int clockPin=9;
int dataPin=12;
int wait=500;

byte LED=0b11001100;
byte LED2=0b11010010;
byte LED3=0b10101010;

void setup() {
  Serial.begin(9600);
    pinMode(latchPin, OUTPUT);
    pinMode(clockPin, OUTPUT);
    pinMode(dataPin, OUTPUT);

}

void loop() {
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin,LSBFIRST, LED);
  digitalWrite(latchPin, HIGH);
  delay(wait);
  
  LED=255-LED;
  
}