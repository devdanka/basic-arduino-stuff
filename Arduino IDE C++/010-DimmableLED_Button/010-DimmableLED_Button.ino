int readPinUp=A0;
int redPin=3;
int readUp;
int brightness=0;
int wait=10;

void setup()  {
  pinMode(readPinUp, INPUT);
  pinMode(redPin, OUTPUT);
  Serial.begin(9600);

}


void loop() {
  readUp=digitalRead(readPinUp);
  Serial.println(brightness);
  if (readUp==0)  {
    brightness=brightness+1;
    if (brightness>=254)  {
      brightness=255;
    }
  }
  else{
    brightness=brightness-1;
    if (brightness<=1) {
      brightness=0;
    }
  }
  analogWrite(redPin, brightness);
  delay(wait);

}