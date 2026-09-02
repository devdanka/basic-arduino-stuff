// IR remote button hex codes:
// 0xBA45FF00 = Button 1
// 0xB946FF00 = Button 2
// 0xB847FF00 = Button 3
// 0xBB44FF00 = Button 4
// 0xBF40FF00 = Button 5
// 0xBC43FF00 = Button 6
// 0xF807FF00 = Button 7
// 0xEA15FF00 = Button 8
// 0xF609FF00 = Button 9
// 0xE619FF00 = Button 0
// 0xE718FF00 = Up arrow (+)
// 0xAD52FF00 = Down arrow (-)
// 0xA55AFF00 = Right arrow (*)
// 0xF708FF00 = Left arrow (/)
// 0xE31CFF00 = OK (calculate)

#include <Arduino.h>
#include <IRremote.hpp>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

const int RECEIVE_PIN = 4;
String expression;
bool showingResult = false;

void showExpression() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(expression);
}

void addDigit(char digit) {
  if (showingResult) {
    expression = "";
    showingResult = false;
  }

  if (expression.length() < 15) {
    expression += digit;
    showExpression();
  }
}

void addOperator(char operation) {
  if (showingResult) {
    showingResult = false;
  }

  if (expression.length() > 0 && expression.indexOf('+') == -1 &&
      expression.indexOf('-') == -1 && expression.indexOf('*') == -1 &&
      expression.indexOf('/') == -1 && expression.length() < 15) {
    expression += operation;
    showExpression();
  }
}

void calculate() {
  int operationIndex = -1;
  char operation = 0;
  int expressionLength = expression.length();

  for (int index = 0; index < expressionLength; index++) {
    char current = expression[index];
    if (current == '+' || current == '-' || current == '*' || current == '/') {
      operationIndex = index;
      operation = current;
      break;
    }
  }

  if (operationIndex <= 0 || operationIndex >= expressionLength - 1) {
    lcd.clear();
    lcd.print("Enter equation");
    return;
  }

  long firstNumber = expression.substring(0, operationIndex).toInt();
  long secondNumber = expression.substring(operationIndex + 1).toInt();
  long result = 0;

  if (operation == '+') result = firstNumber + secondNumber;
  if (operation == '-') result = firstNumber - secondNumber;
  if (operation == '*') result = firstNumber * secondNumber;
  if (operation == '/') {
    if (secondNumber == 0) {
      lcd.clear();
      lcd.print("Cannot divide");
      lcd.setCursor(0, 1);
      lcd.print("by zero");
      return;
    }
    result = firstNumber / secondNumber;
  }

  lcd.clear();
  lcd.print(expression);
  lcd.setCursor(0, 1);
  lcd.print("= ");
  lcd.print(result);
  expression = String(result);
  showingResult = true;
}

void setup() {
  Serial.begin(9600);

  lcd.init();
  lcd.backlight();
  lcd.print("IR Calculator");
  delay(1000);
  lcd.clear();
  IrReceiver.begin(RECEIVE_PIN, ENABLE_LED_FEEDBACK);
}

void loop() {
  if (IrReceiver.decode()) {

    uint32_t code = IrReceiver.decodedIRData.decodedRawData;
    Serial.print("0x");
    Serial.println(code, HEX);
    const char* button = nullptr;

    switch (code) {
      case 0xBA45FF00: button = "1"; break;
      case 0xB946FF00: button = "2"; break;
      case 0xB847FF00: button = "3"; break;
      case 0xBB44FF00: button = "4"; break;
      case 0xBF40FF00: button = "5"; break;
      case 0xBC43FF00: button = "6"; break;
      case 0xF807FF00: button = "7"; break;
      case 0xEA15FF00: button = "8"; break;
      case 0xF609FF00: button = "9"; break;
      case 0xE619FF00: button = "0"; break;
    }

    if (button != nullptr) {
      addDigit(button[0]);
    } else if (code == 0xE718FF00) {
      addOperator('+');
    } else if (code == 0xAD52FF00) {
      addOperator('-');
    } else if (code == 0xA55AFF00) {
      addOperator('*');
    } else if (code == 0xF708FF00) {
      addOperator('/');
    } else if (code == 0xE31CFF00) {
      calculate();
    }

    IrReceiver.resume();
  }
}