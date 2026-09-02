# 002 — Photo-detecting servo

Wave your hand over the photoresistor and the servo swings. Light level in,
servo angle out.

**Source:** [`002-PhotoDetectingServo.cpp`](002-PhotoDetectingServo.cpp)
**Build:** `pio run -e servo -t upload`

## Parts

- Arduino Uno
- SG90 micro servo
- Photoresistor (LDR)
- 10 kΩ resistor
- Breadboard and jumper wires

## Wiring

| Component | Arduino pin |
|---|---|
| Servo signal | D6 |
| Servo power / ground | 5V / GND |
| LDR — one leg | 5V |
| LDR — other leg | A0 **and** one leg of the 10 kΩ resistor |
| 10 kΩ resistor — other leg | GND |

## How it works

```cpp
servoPos = (analogRead(readPin) - 970) * 180 / 53;
```

This is a hand-calibrated map. It assumes the LDR reads about **970** in the
dark and about **1023** in the light — a span of only 53 counts out of 1023 —
and stretches that narrow band across the full 0–180° of servo travel.

That's a very tight window, and it's specific to this LDR, this resistor and
this room. If the servo just slams to one end and stays there, the calibration
is off for your setup. Fix it by watching the raw numbers first:

```bash
pio device monitor -e servo
```

Note the value with the LDR covered and the value with a torch on it, then put
those two numbers into the formula in place of 970 and 53 (`53` is
`bright − dark`). Arduino's built-in `map(value, dark, bright, 0, 180)` does
the same arithmetic more readably.

The clamps afterwards stop an out-of-range reading from being written to the
servo:

```cpp
if (servoPos < 0)        servoPos = 0;
else if (servoPos > 180) servoPos = 180;
```

## Run it

```bash
pio run -e servo -t upload
pio device monitor -e servo
```

The monitor prints both the raw analog value and the angle it produced, one
line per second.
