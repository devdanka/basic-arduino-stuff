#include <Arduino.h>
#include <Servo.h>

int servoPin=6;
int servoPos;
int readPin=A0;
int wait=1000;
Servo myServo;

void setup() {
    pinMode(readPin, INPUT);
    myServo.attach(servoPin);
    Serial.begin(9600);
}

void loop() {
    servoPos = (analogRead(readPin) - 970) * 180 / 53;
    if (servoPos < 0) {
        servoPos = 0;
    } else if (servoPos > 180) {
        servoPos = 180;
    }
    myServo.write(servoPos);
    Serial.println("Analog value: " + String(analogRead(readPin)));
    Serial.println("Servo position set to: " + String(servoPos));
    delay(wait); 
}