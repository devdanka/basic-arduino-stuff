int Pin=5;
int wait=100;
int read=A1;
float result;
float volt;
int LEDvolt;

void setup() {
  // put your setup code here, to run once:
  pinMode(read, INPUT);
  pinMode(Pin, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  result=analogRead(read);
  volt=(5./1023.)*result;
  LEDvolt=(255./1023.)*result;
  Serial.println(volt);
  analogWrite(Pin, LEDvolt);
  delay(wait);
}