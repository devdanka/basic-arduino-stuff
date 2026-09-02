int readPin=3;
int buttonPin=11;
int redPin=6;
int buttonState=0;
int newL;
int old=1;
int wait=100;

void setup() {
  // put your setup code here, to run once:
  pinMode(buttonPin, OUTPUT);
  pinMode(redPin, OUTPUT);
  pinMode(readPin, INPUT);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  newL=digitalRead(readPin);
  Serial.println(newL);
  if(old==0 && newL==1)  {
    if (buttonState==0) {
      digitalWrite(redPin, HIGH);
      buttonState=1;
    }
    else{
      digitalWrite(redPin, LOW);
      buttonState=0;
    }
  }
  old=newL;
  delay(wait);
}
