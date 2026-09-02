int redPin=10;
int readPin=A1;
int read;
int wait=200;

void setup() {
  // put your setup code here, to run once:
  pinMode(redPin, OUTPUT);
  pinMode(readPin, INPUT);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  read=analogRead(readPin);
  Serial.println(read);
  delay(wait);

  while (read<800)  {
    digitalWrite(redPin, HIGH);
    read=analogRead(readPin);
    Serial.println(read);
    delay(wait);
  }
  digitalWrite(redPin, LOW);

}
