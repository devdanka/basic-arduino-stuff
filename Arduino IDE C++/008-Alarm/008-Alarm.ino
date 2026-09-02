int buzzPin=3;
int readPin=A0;
int mainPin=11;
int redPin=6;
int read;
int wait=10;
int waitT=100;


void setup() {
  // put your setup code here, to run once:
  pinMode(buzzPin, OUTPUT);
  pinMode(readPin, INPUT);
  pinMode(mainPin, OUTPUT);
  pinMode(redPin, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(mainPin, HIGH);
  read=analogRead(readPin);
  Serial.println(read);

  if (read>1000)  {
    tone(buzzPin, 1000);
    digitalWrite(redPin, HIGH);
    delay(waitT);
    noTone(buzzPin);
    digitalWrite(redPin, LOW);
    delay(waitT);
  }

  if (read<1000)  {

  }
  
  delay(wait);
}
