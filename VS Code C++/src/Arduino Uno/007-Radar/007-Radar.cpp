// 007-Radar - HC-SR04 on a sweeping servo, streamed to the Radar app.
//
// This is 005-Ultrasonic (the distance reading) bolted onto 006-ServoSweep
// (the motion). The sensor rides the servo horn and looks wherever the horn
// points, so every reading has a bearing as well as a range - which is all a
// radar screen needs.
//
// Wiring:
//   HC-SR04  trig -> D10 , echo -> D9 , VCC -> 5V , GND -> GND
//   Servo    signal -> D6 , red -> 5V , brown -> GND
//   Mount the sensor on the servo horn, facing away from the shaft.
//
// D6 is fine for the servo even though the sensor sits on D9/D10: the Servo
// library takes over Timer1, which only costs you analogWrite() on 9 and 10,
// and those two are used here as plain digital pins for trig and echo.
//
// Serial output, 9600 baud, one line per reading:
//   <angle>,<distance_cm>       e.g.  90,42.3
//   distance is -1 when nothing echoed back (out of range / bad angle)
// Lines starting with '#' are comments and are ignored by the app.
//
// No LCD here on purpose: an I2C write between every step would stall the
// sweep and make the screen jerky. The screen IS the display now.
//
// Build/upload:  PlatformIO env "radar"

#include <Arduino.h>
#include <Servo.h>

const int trigPin  = 10;
const int echoPin  = 9;
const int servoPin = 6;      // signal wire; 005/006 used A1, this one is on D6

const int ANGLE_MIN = 15;    // servos rarely behave at the last few degrees
const int ANGLE_MAX = 165;
const int STEP      = 1;     // degrees per reading
const int SETTLE_MS = 25;    // let the horn arrive before pinging

// pulseIn timeout. Sound covers ~29.1 us per cm each way, so 25 ms is about
// 4.3 m - past the HC-SR04's honest range, and it stops a missing echo from
// blocking the sweep for the full default second.
const unsigned long ECHO_TIMEOUT_US = 25000UL;

Servo radarServo;

// One ping. Returns cm, or -1 if nothing came back.
float pingCm() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  unsigned long duration = pulseIn(echoPin, HIGH, ECHO_TIMEOUT_US);
  if (duration == 0) return -1.0;          // timed out
  return (duration / 2.0) / 29.1;          // same maths as 005-Ultrasonic
}

void report(int angle) {
  float d = pingCm();
  Serial.print(angle);
  Serial.print(',');
  Serial.println(d, 1);
}

void setup() {
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  radarServo.attach(servoPin);

  radarServo.write(ANGLE_MIN);
  delay(400);                              // one long move before the sweep
  Serial.println("# radar ready");
}

void loop() {
  for (int a = ANGLE_MIN; a <= ANGLE_MAX; a += STEP) {
    radarServo.write(a);
    delay(SETTLE_MS);
    report(a);
  }
  for (int a = ANGLE_MAX; a >= ANGLE_MIN; a -= STEP) {
    radarServo.write(a);
    delay(SETTLE_MS);
    report(a);
  }
}
