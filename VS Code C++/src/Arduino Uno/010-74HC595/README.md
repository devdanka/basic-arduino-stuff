# 010 — Shift register

Drives **8 LEDs from 3 pins** using a 74HC595 shift register. The pattern
inverts every half second, so the LEDs alternate.

**Source:** [`010-74HC595.cpp`](010-74HC595.cpp)
**Build:** `pio run -e 74hc595 -t upload`

## Parts

- Arduino Uno
- 74HC595 shift register (16-pin DIP)
- 8 × LEDs
- 8 × 220 Ω resistors
- Breadboard and jumper wires

## Wiring

| 74HC595 pin | Name | Goes to |
|---|---|---|
| 14 | DS (data) | D12 |
| 11 | SHCP (clock) | D9 |
| 12 | STCP (latch) | D11 |
| 13 | OE | GND |
| 10 | MR | 5V |
| 16 | VCC | 5V |
| 8 | GND | GND |
| 15, 1–7 | Q0–Q7 | 8 LEDs, each through a 220 Ω resistor to GND |

Pins 13 (OE, output enable) and 10 (MR, master reset) are both **active low**.
Tying OE to GND keeps the outputs switched on; tying MR to 5V keeps the chip
out of reset. Leave either floating and the chip looks dead.

Output Q0 is on pin 15, then Q1–Q7 run along pins 1 to 7. Slightly awkward
numbering — check the datasheet against your breadboard.

## How it works

The 74HC595 is a **serial-in, parallel-out** register. You clock 8 bits in one
at a time down a single wire, then pulse the latch and all 8 outputs change at
once.

```cpp
digitalWrite(latchPin, LOW);                       // hold the outputs steady
shiftOut(dataPin, clockPin, LSBFIRST, LED);        // clock 8 bits in
digitalWrite(latchPin, HIGH);                      // release - all 8 update
```

Keeping the latch LOW during the shift is what stops the LEDs from flickering
through intermediate patterns while the bits are on their way in.

`shiftOut` does the per-bit work: set the data pin, pulse the clock, repeat 8
times. `LSBFIRST` sends the rightmost bit of the byte first, so `0b11001100`
lands on the outputs mirrored — swap it for `MSBFIRST` if the pattern comes out
backwards from what you drew.

The pattern is written in **binary literal** form, which makes the LED layout
readable at a glance:

```cpp
byte LED = 0b11001100;   // on on off off on on off off
```

`LED = 255 - LED` flips every bit — 255 is `0b11111111`, so subtracting turns
each 1 into a 0 and vice versa.

## Note

`platformio.ini` pulls in the `ShiftRegister74HC595` library, but this sketch
doesn't use it — `shiftOut()` is built into the Arduino core. `LED2` and `LED3`
are declared but never used either; they're spare patterns to try.

## Chaining

The whole point of these chips is that Q7' (pin 9) feeds the next chip's DS.
Two 595s give you 16 outputs, three give 24 — still from the same 3 Arduino
pins.

## Run it

```bash
pio run -e 74hc595 -t upload
```
