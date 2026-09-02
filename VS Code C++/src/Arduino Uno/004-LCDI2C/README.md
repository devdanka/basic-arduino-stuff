# 004 — Keypad calculator

A working four-function calculator. Punch in a number on the 4×4 keypad, press
`A`, punch in the second number, press `A`, then press one of five buttons to
pick the operation. The answer shows on the LCD until you press *next*.

**Source:** [`004-LCDI2C.cpp`](004-LCDI2C.cpp)
**Build:** `pio run -e lcd-i2c -t upload`

## Parts

- Arduino Uno
- 4×4 membrane keypad
- 16×2 LCD with I²C backpack (address 0x27)
- 5 × push buttons
- Breadboard and jumper wires

No resistors needed for the buttons — they use the ATmega's internal pull-ups.

## Wiring

**Keypad** (8 pins, left to right as you look at the ribbon):

| Keypad | Arduino pin |
|---|---|
| Row 1 | D7 |
| Row 2 | D6 |
| Row 3 | D5 |
| Row 4 | D4 |
| Col 1 | D3 |
| Col 2 | D2 |
| Col 3 | A0 |
| Col 4 | A1 |

**Operation buttons** — each wired between its pin and GND:

| Button | Arduino pin |
|---|---|
| Next / clear | D13 |
| Add `+` | D12 |
| Subtract `−` | D11 |
| Multiply `×` | D10 |
| Divide `÷` | D9 |

**LCD:** SDA → A4, SCL → A5, VCC → 5V, GND → GND.

## Why columns 3 and 4 are on A0/A1

They used to be on **D0** and **D1** — which are the RX/TX pins the USB-to-serial
chip uses to talk to the board. Sharing them with a keypad causes real trouble:
uploads fail while the keypad is connected, pressing a key during a reset
confuses the bootloader, and `Serial.begin()` can't be used at all.

The fix is free. The analog pins work as ordinary digital pins (A0 is also known
as D14), and `Keypad` doesn't care which pins it gets:

```cpp
byte colPins[COLS] = {3, 2, A0, A1};
```

D8, A2 and A3 are still free if you need more.

## How it works

A keypad is a grid, not 16 separate switches. The library drives one row low at
a time and checks which column reads low — 16 buttons from 8 pins.

The buttons use `INPUT_PULLUP`, so the pin idles HIGH and reads LOW when
pressed. That's the internal ~20 kΩ resistor doing the job of an external
pull-up.

Each entry stage blocks in a `while (true)` loop until `A` is pressed, so the
calculator moves through a strict sequence: number → number → operation →
result → wait for *next*.

Division by zero is caught explicitly and shows `Error: Div by 0` rather than
crashing.

## Why `result` is a float

`result` used to be an `int`, and `7 ÷ 2` displayed as `3`. The sketch was
already casting to float on the division line —
`result = (float)number / number2;` — but that cast happens *before* the value
is stored, and storing a float into an `int` throws the decimals away. The cast
was doing nothing.

It's a `float` now, with the display doing the rounding instead of the type:

```cpp
if (result == (long)result) {
    lcd.print((long)result);   // 5, not 5.00
} else {
    lcd.print(result, 2);      // 3.50
}
```

Whole answers stay clean and only real remainders show decimals.

## Run it

```bash
pio run -e lcd-i2c -t upload
```
