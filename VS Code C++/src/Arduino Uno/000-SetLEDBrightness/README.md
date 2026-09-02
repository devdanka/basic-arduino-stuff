# 000 — Set LED brightness

Two buttons: one ramps the LED brighter, the other ramps it dimmer. The
two-button version of `010-DimmableLED_Button` from the Arduino IDE folder.

**Source:** [`000-SetLEDBrightness.cpp`](000-SetLEDBrightness.cpp)
**Build:** `pio run -e led -t upload`

## Parts

- Arduino Uno
- 2 × push buttons
- 2 × 10 kΩ resistors (pull-ups)
- Red LED + 220 Ω resistor
- Breadboard and jumper wires

## Wiring

| Component | Arduino pin |
|---|---|
| Button "up" — one side | GND |
| Button "up" — other side | A0 + 10 kΩ to 5V |
| Button "down" — one side | GND |
| Button "down" — other side | A2 + 10 kΩ to 5V |
| Red LED (+ 220 Ω) | D3 |
| LED cathode | GND |

Both buttons are **active low** — the sketch tests for `== 0`. D3 has to be a
PWM pin for `analogWrite` to work.

## How it works

Each pass of `loop()` nudges `brightness` by one and writes it out:

```cpp
if (readUp   == 0) brightness = brightness + 1;
if (readDown == 0) brightness = brightness - 1;
analogWrite(redPin, brightness);
```

`wait` is only 1 ms here (the Arduino IDE version used 10), so a full sweep
from 0 to 255 takes about a quarter of a second — noticeably twitchier.

Holding both buttons at once cancels out and the brightness stays put, since
the two `if`s add and subtract on the same pass.

## Run it

```bash
pio run -e led -t upload
pio device monitor -e led
```

The monitor prints the current brightness value at 9600 baud.
