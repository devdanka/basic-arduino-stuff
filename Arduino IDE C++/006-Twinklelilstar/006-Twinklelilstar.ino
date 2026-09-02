int number;
int buzzerPin=3;
int wait=200;
int longwait=wait*3;
int j;
int Do=261;
int Re=293;
int Mi=329;
int Fa=349;
int Sol=392;
int La=440;
int Si=493;
int Do2=522; 
void setup() {
  // put your setup code here, to run once:
  pinMode(buzzerPin, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  tone(buzzerPin, Do);
  delay(wait);
  noTone(buzzerPin);
  delay(wait);

  tone(buzzerPin, Do);
  delay(wait);
  noTone(buzzerPin);
  delay(wait);
  
  tone(buzzerPin, Sol);
  delay(wait);
  noTone(buzzerPin);
  delay(wait);
  
  tone(buzzerPin, Sol);
  delay(wait);
  noTone(buzzerPin);
  delay(wait);
  
  tone(buzzerPin, La);
  delay(wait);
  noTone(buzzerPin);
  delay(wait);
  
  tone(buzzerPin, La);
  delay(wait);
  noTone(buzzerPin);
  delay(wait);
  
  tone(buzzerPin, Sol);
  delay(wait);
  noTone(buzzerPin);
  delay(longwait);

  tone(buzzerPin, Fa);
  delay(wait);
  noTone(buzzerPin);
  delay(wait);

  tone(buzzerPin, Fa);
  delay(wait);
  noTone(buzzerPin);
  delay(wait);

  tone(buzzerPin, Mi);
  delay(wait);
  noTone(buzzerPin);
  delay(wait);

  tone(buzzerPin, Mi);
  delay(wait);
  noTone(buzzerPin);
  delay(wait);

  tone(buzzerPin, Re);
  delay(wait);
  noTone(buzzerPin);
  delay(wait);

  tone(buzzerPin, Re);
  delay(wait);
  noTone(buzzerPin);
  delay(wait);

  tone(buzzerPin, Do);
  delay(wait);
  noTone(buzzerPin);
  delay(longwait);
}
