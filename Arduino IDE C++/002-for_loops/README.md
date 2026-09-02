# 002 — For loops

Practice with `for` loops. The red LED blinks 5 times, then the yellow LED
blinks 4 times, then it starts over.

## Parts

- Arduino Uno
- Red and yellow LED
- 2 × 220 Ω resistors
- Breadboard and jumper wires

## Wiring

| Component | Arduino pin |
|---|---|
| Red LED (+ 220 Ω) | D10 |
| Yellow LED (+ 220 Ω) | D6 |
| Both LED cathodes | GND |

## How it works

Each loop counts a variable `j` from 1 up to a repeat count, and each pass does
one on/off cycle:

```cpp
for (j = 1; j <= redRepeat; j = j + 1) {
  digitalWrite(redPin, HIGH);
  delay(wait);
  digitalWrite(redPin, LOW);
  delay(wait);
}
```

Change `redRepeat`, `yellowRepeat` or `wait` at the top of the file to change
the pattern — no need to touch the loops themselves. That's the point of the
exercise.

## Why both pins need `pinMode`

`setup()` has to declare *both* LEDs as outputs. An earlier version of this
sketch only declared the yellow one, and the red LED still lit — just very
faintly. That's because `digitalWrite(pin, HIGH)` on a pin left as an input
switches on the internal pull-up resistor instead of driving the pin, and
~20 kΩ leaks just enough current to make an LED glow dimly.

A dim LED where you expected a bright one is nearly always a missing
`pinMode(pin, OUTPUT)`.

## Run it

Open the `.ino` in the Arduino IDE, select *Arduino Uno* and the COM port, and
click Upload.
