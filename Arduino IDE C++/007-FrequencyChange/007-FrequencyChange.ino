int potentioPin=11;
int buzzPin=3;
int readPin=A0;
int read;
int note;
float volt;
int wait=10;

void setup() {
  // put your setup code here, to run once:
  pinMode(buzzPin, OUTPUT);
  pinMode(potentioPin, OUTPUT);
  pinMode(readPin, INPUT);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(buzzPin, HIGH);
  digitalWrite(potentioPin, HIGH);
  
  read=analogRead(readPin);
  volt=(5./1023.)*read;
  note=(9940./1023.)*read;  // decimal points matter: 9940/1023 as ints is just 9
  
  tone(buzzPin, note);
  Serial.println(volt);
  
  delay(wait);
}
