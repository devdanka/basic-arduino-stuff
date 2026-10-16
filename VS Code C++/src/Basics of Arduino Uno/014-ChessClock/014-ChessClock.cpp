#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

const byte SWITCH_PIN = 2;
const unsigned long START_TIME_MS = 5UL * 60UL * 1000UL;
const unsigned long DEBOUNCE_MS = 30;

unsigned long remaining[2] = {START_TIME_MS, START_TIME_MS};
unsigned long lastTick = 0;
unsigned long lastSwitchChange = 0;
unsigned long lastDisplay = 0;
byte activePlayer = 0;
bool lastSwitchOn = false;
bool stableSwitchOn = false;
bool gameOver = false;

void showClock() {
    for (byte player = 0; player < 2; player++) {
        unsigned long seconds = (remaining[player] + 999UL) / 1000UL;
        char divider = player == 0 ? '|' : ' ';
        char line[9];
        snprintf(line, sizeof(line), "%c%-5s %c",
                 !gameOver && player == activePlayer ? '>' : ' ',
                 player == 0 ? "WHITE" : "BLACK", divider);
        lcd.setCursor(player * 8, 0);
        lcd.print(line);
        if (gameOver) {
            snprintf(line, sizeof(line), " %-5s %c",
                     remaining[player] == 0 ? "LOST" : "WIN", divider);
        } else {
            snprintf(line, sizeof(line), " %02lu:%02lu %c", seconds / 60, seconds % 60, divider);
        }
        lcd.setCursor(player * 8, 1);
        lcd.print(line);
    }
}

void setup() {
    pinMode(SWITCH_PIN, INPUT_PULLUP);
    lcd.init();
    lcd.backlight();
    lastSwitchOn = stableSwitchOn = digitalRead(SWITCH_PIN) == LOW;
    activePlayer = stableSwitchOn ? 0 : 1;
    showClock();
    lastTick = lastSwitchChange = lastDisplay = millis();
}

void loop() {
    if (gameOver) return;

    unsigned long now = millis();
    unsigned long elapsed = now - lastTick;
    lastTick = now;

    if (elapsed >= remaining[activePlayer]) {
        remaining[activePlayer] = 0;
        gameOver = true;
        showClock();
        return;
    }
    remaining[activePlayer] -= elapsed;

    bool switchOn = digitalRead(SWITCH_PIN) == LOW;
    if (switchOn != lastSwitchOn) {
        lastSwitchOn = switchOn;
        lastSwitchChange = now;
    }

    bool turnChanged = switchOn != stableSwitchOn && now - lastSwitchChange >= DEBOUNCE_MS;
    if (turnChanged) {
        stableSwitchOn = switchOn;
        activePlayer = stableSwitchOn ? 0 : 1;
    }

    if (turnChanged || now - lastDisplay >= 100) {
        lastDisplay = now;
        showClock();
    }
}
