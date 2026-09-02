int redPin=10;
int yellowPin=6u;
int redRepeat=5;
int yellowRepeat=4;
int wait=100;
int j;

void setup() {
  // put your setup code here, to run once:
  pinMode(redPin, OUTPUT);
  pinMode(yellowPin, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  for(j=1;j<=redRepeat;j=j+1) {
    digitalWrite(redPin, HIGH);
    delay(wait);
    digitalWrite(redPin, LOW);
    delay(wait);
  }

  for(j=1;j<=yellowRepeat;j=j+1)  {
    digitalWrite(yellowPin, HIGH);
    delay(wait);
    digitalWrite(yellowPin, LOW);
    delay(wait);
  }
}
