// IR remote button hex codes:
// 0xBA45FF00 = Button 1
// 0xB946FF00 = Button 2
// 0xB847FF00 = Button 3
// 0xBB44FF00 = Button 4
// 0xBF40FF00 = Button 5
// 0xBC43FF00 = Button 6
// 0xF807FF00 = Button 7
// 0xEA15FF00 = Button 8
// 0xF609FF00 = Button 9
// 0xE619FF00 = Button 0
// 0xE718FF00 = Up arrow (+)
// 0xAD52FF00 = Down arrow (-)
// 0xA55AFF00 = Right arrow (*)
// 0xF708FF00 = Left arrow (/)
// 0xE31CFF00 = OK 

#include <Arduino.h>
#include <IRremote.hpp>
#include <Servo.h>

Servo myservo;

const int RECEIVE_PIN = 4;
const int SERVO_PIN = 6;
const int SERVO_STEP = 5;
const unsigned long MOVE_INTERVAL = 60;
const unsigned long PRESS_DEBOUNCE = 350;
int servoPosition = 90;
int heldDirection = 0;
unsigned long lastMoveTime = 0;
uint32_t lastPressCode = 0;
unsigned long lastFrameTime = 0;

void setup() {
	Serial.begin(9600);
	myservo.attach(SERVO_PIN);
	myservo.write(servoPosition);
	IrReceiver.begin(RECEIVE_PIN, ENABLE_LED_FEEDBACK);
	Serial.println("IR servo remote ready");
}

void loop() {
	if (IrReceiver.decode()) {
		uint32_t code = IrReceiver.decodedIRData.decodedRawData;

		bool isKnownButton = code == 0xA55AFF00 || code == 0xF708FF00;
		bool isNewPress = code != lastPressCode ||
			millis() - lastFrameTime >= PRESS_DEBOUNCE;

		if (isKnownButton && isNewPress) {
			if (code == 0xA55AFF00) {
				heldDirection = heldDirection == 1 ? 0 : 1;
			} else if (code == 0xF708FF00) {
				heldDirection = heldDirection == -1 ? 0 : -1;
			}
			lastPressCode = code;
			Serial.print("Direction: ");
			Serial.println(heldDirection);
		}
		if (isKnownButton) {
			lastFrameTime = millis();
		}

		if (isKnownButton) {
			Serial.print("0x");
			Serial.println(code, HEX);
		}
		IrReceiver.resume();
	}

	if (heldDirection != 0 &&
	    millis() - lastMoveTime >= MOVE_INTERVAL) {
		servoPosition = constrain(servoPosition + heldDirection * SERVO_STEP, 0, 180);
		myservo.write(servoPosition);
		lastMoveTime = millis();
	}
}

