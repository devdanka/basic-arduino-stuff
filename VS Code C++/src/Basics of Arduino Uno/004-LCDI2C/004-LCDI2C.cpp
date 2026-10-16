#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {7, 6 , 5, 4};
// Columns 3 and 4 are on A0/A1, not D1/D0. The analog pins work as ordinary
// digital pins, and D0/D1 are the USB serial pins - a keypad on those fights
// the bootloader and makes uploads fail.
byte colPins[COLS] = {3, 2, A0, A1};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

const int PIN_NEXT = 13;
const int PIN_ADD  = 12;
const int PIN_SUB  = 11;
const int PIN_MUL  = 10;
const int PIN_DIV  = 9;

void setup() {
    lcd.init();          
    lcd.backlight();     
    
    pinMode(PIN_NEXT, INPUT_PULLUP);
    pinMode(PIN_ADD, INPUT_PULLUP);
    pinMode(PIN_SUB, INPUT_PULLUP);
    pinMode(PIN_MUL, INPUT_PULLUP);
    pinMode(PIN_DIV, INPUT_PULLUP);
}

void loop() {
    long number = 0;
    long number2 = 0;
    float result = 0;  // float, or division would truncate 7/2 to 3
    String operation = "";
    String inputString = "";

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Enter number 1:");
    lcd.setCursor(0, 1);
    
    inputString = "";
    while (true) {
        char key = keypad.getKey();
        if (key) {
            if (key >= '0' && key <= '9') {
                inputString = inputString + key;
                lcd.print(key); 
            } else if (key == 'A') {
                if (inputString.length() > 0) {
                    number = inputString.toInt();
                    break; 
                }
            }
        }
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Enter number 2:");
    lcd.setCursor(0, 1);
    
    inputString = "";
    while (true) {
        char key = keypad.getKey();
        if (key) {
            if (key >= '0' && key <= '9') {
                inputString = inputString + key;
                lcd.print(key); 
            } else if (key == 'A') {
                if (inputString.length() > 0) {
                    number2 = inputString.toInt();
                    break; 
                }
            }
        }
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Select operation:");

    while (operation == "") {
        if (digitalRead(PIN_ADD) == LOW) {
            operation = "+";
        } else if (digitalRead(PIN_SUB) == LOW) {
            operation = "-";
        } else if (digitalRead(PIN_MUL) == LOW) {
            operation = "*";
        } else if (digitalRead(PIN_DIV) == LOW) {
            operation = "/";
        }
        delay(20);
    }

    delay(200);

    if (operation == "+") {
        result = number + number2;
    } else if (operation == "-") {
        result = number - number2;
    } else if (operation == "*") {
        result = number * number2;
    } else if (operation == "/") {
        if (number2 != 0) {
            result = (float)number / number2;
        } else {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Error: Div by 0");
            delay(2000);
            return; 
        }
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(number);
    lcd.print(operation);
    lcd.print(number2);
    lcd.setCursor(0, 1);
    lcd.print("= ");
    // Whole answers print as "5" rather than "5.00"; only division that
    // actually leaves a remainder shows decimals.
    if (result == (long)result) {
        lcd.print((long)result);
    } else {
        lcd.print(result, 2);
    }

    delay(500); 
    
    while (digitalRead(PIN_NEXT) == HIGH) {
        delay(50);
    }
    delay(300); 
}