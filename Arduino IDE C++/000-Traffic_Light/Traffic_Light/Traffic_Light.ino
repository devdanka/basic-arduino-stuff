int redLED=12;
int yellowLED=10;
int greenLED=6;
int greenBrightness=10;
int longW=1000;
int shortW=500;

void setup() {
  // put your setup code here, to run once:
  pinMode(redLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);
  pinMode(greenLED, OUTPUT);

}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(yellowLED, LOW);
  digitalWrite(redLED, HIGH);
  delay(longW);

  digitalWrite(redLED, LOW);
  digitalWrite(yellowLED, HIGH);
  delay(shortW);

  digitalWrite(yellowLED, LOW);
  analogWrite(greenLED, greenBrightness);
  delay(longW);

  analogWrite(greenLED, 0);
  digitalWrite(yellowLED, HIGH);
  delay(shortW);
}


