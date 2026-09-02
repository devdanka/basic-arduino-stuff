#include <Arduino.h>
#include <Servo.h>

int servoPin = A1;
int servoPos;
int stepSize = 1;  
int stepDelay = 15;
Servo myServo;

void setup() {
    myServo.attach(servoPin);
}

void loop() {
    for (servoPos = 0; servoPos <= 180; servoPos += stepSize) {
        myServo.write(servoPos);
        delay(stepDelay);
    }

    for (servoPos = 180; servoPos >= 0; servoPos -= stepSize) {
        myServo.write(servoPos);
        delay(stepDelay);
    }
}
