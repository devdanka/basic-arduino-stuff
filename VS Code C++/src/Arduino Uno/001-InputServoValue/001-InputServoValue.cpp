#include <Arduino.h>
#include <Servo.h>

int servoPin=6;
int servoPos;
Servo myServo;

void setup() {
    myServo.attach(servoPin);
    Serial.begin(9600);
}

void loop() {
    Serial.println("Enter the servo position (0-180): ");
    while (Serial.available() == 0) {
    }
    servoPos = Serial.parseInt();
    if (servoPos >= 0 && servoPos <= 180) {
        myServo.write(servoPos);
        Serial.println("Servo position set to: " + String(servoPos));
    } else {
        Serial.println("Invalid position. Please enter a value between 0 and 180.");
    }
    Serial.println();
}