#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 oled(128, 64, &Wire, -1);

void setup() {
    Serial.begin(115200);
    Wire.begin();

    if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("OLED initialization failed");
        while (true) {
            delay(1000);
        }
    }

    oled.clearDisplay();
    oled.setTextColor(SSD1306_WHITE);

    oled.setTextSize(2);
    oled.setCursor(0, 0);
    oled.println("Hello!");

    oled.setTextSize(1);
    oled.setCursor(0, 30);
    oled.println("My OLED is working!");
    oled.println("Change this text.");

    oled.display();
}

void loop() {
}