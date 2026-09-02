#include <Arduino.h>
#include <IRremote.h>

int pinRelay = 6;
int IRsensor = 12;
bool relayState = false; 

void setup() {
  pinMode(pinRelay, OUTPUT);
  digitalWrite(pinRelay, HIGH); 
  Serial.begin(9600);
  IrReceiver.begin(IRsensor, ENABLE_LED_FEEDBACK);
}

void loop() {
  if (IrReceiver.decode()) {
    unsigned long commandCode = IrReceiver.decodedIRData.command;
    if (IrReceiver.decodedIRData.decodedRawData == 0xE31CFF00 || IrReceiver.decodedIRData.command == 0x34) {
      relayState = !relayState; 
      if (relayState) {
        digitalWrite(pinRelay, LOW); 
      } else {
        digitalWrite(pinRelay, HIGH);
      }
    }
    IrReceiver.resume();
  }
}