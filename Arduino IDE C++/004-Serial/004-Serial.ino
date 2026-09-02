int number;
int redPin=10;
String msg="How many times do you want the red LED to blink? ";
int wait=300;
int j;

void setup() {
  // put your setup code here, to run once:
  pinMode(redPin, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println();
  Serial.println(msg);
  while (Serial.available()==0) {

  }

  number=Serial.parseInt();
  for(j=1;j<=number;j=j+1)  {
    digitalWrite(redPin, HIGH);
    delay(wait);
    digitalWrite(redPin, LOW);
    delay(wait);
  }
}
