// Emits named events over serial for Trigger.exe to turn into keystrokes.
//
//   Arduino (this sketch)  --serial 9600-->  Trigger.exe  --SendInput-->  Windows
//        "DARK" / "IR:E31CFF00"                triggers.txt maps them
//
// Wiring is unchanged from 011/012:
//
//   D5 (HIGH) ---[ LDR ]---+---[ 10k ]--- GND        IR receiver OUT -> D12
//                          |
//                         A0
//
// The relay is deliberately not here - 011 and 012 still drive that. This
// sketch only reports; the PC decides what an event means.

#include <Arduino.h>
#include <IRremote.hpp>

int pinPhotoresistor = A0;
int pinPhotoPower = 5;
int IRsensor = 12;

int threshold = 500;
int margin = 20;

bool covered = false;
int photoresistorValue;

void setup() {
  pinMode(pinPhotoPower, OUTPUT);
  digitalWrite(pinPhotoPower, HIGH);
  Serial.begin(9600);
  IrReceiver.begin(IRsensor, ENABLE_LED_FEEDBACK);
}

void loop() {
  photoresistorValue = analogRead(pinPhotoresistor);

  // Edges, not levels. The PC wants "it just went dark", not "it is dark" -
  // the latter would arrive a few hundred times a second and, mapped to a
  // keystroke, would jam the key down.
  if (!covered && photoresistorValue < threshold - margin) {
    covered = true;
    Serial.println("DARK");
  } else if (covered && photoresistorValue > threshold + margin) {
    covered = false;
    Serial.println("LIGHT");
  }

  if (IrReceiver.decode()) {
    bool isRepeat = IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT;
    if (!isRepeat && IrReceiver.decodedIRData.decodedRawData != 0) {
      Serial.print("IR:");
      Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);
    }
    IrReceiver.resume();
  }

  // No delay(): the IR receiver needs a responsive loop, and nothing is
  // printed unless something actually happened.
}
