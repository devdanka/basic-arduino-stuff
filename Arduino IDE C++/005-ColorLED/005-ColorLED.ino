int redPin=10;
int yellowPin=6;
String msg="What color LED do you want to turn on? (Red/Yellow)";
String color;
int waitTime=1000;

void setup() {
  pinMode(redPin, OUTPUT);
  pinMode(yellowPin, OUTPUT);
  Serial.begin(9600);

}

void loop() {
  Serial.println(msg);
  while (Serial.available()==0) {

  }
  color=Serial.readString();
  color.trim();  // drop the \r\n the Serial Monitor may add, so "Red\n"=="Red"

  if (color=="Red") {
    digitalWrite(redPin, HIGH);
    delay(waitTime);
    digitalWrite(redPin, LOW);
    delay(waitTime);
  }

  if (color=="Yellow")  {
    digitalWrite(yellowPin, HIGH);
    delay(waitTime);
    digitalWrite(yellowPin, LOW);
    delay(waitTime);
  } 

}
