# 000 — Traffic Light

Three LEDs cycling through a real traffic-light sequence: red, red→yellow,
green, yellow, repeat.

## Parts

- Arduino Uno
- Red, yellow and green LED
- 3 × 220 Ω resistors
- Breadboard and jumper wires

## Wiring

| Component | Arduino pin |
|---|---|
| Red LED (+ 220 Ω) | D12 |
| Yellow LED (+ 220 Ω) | D10 |
| Green LED (+ 220 Ω) | D6 |
| All LED cathodes | GND |

Every LED needs its own resistor between the short leg and GND, otherwise it
draws too much current and eventually burns out.

## How it works

The whole sketch is `digitalWrite` and `delay`. The one interesting bit is the
green LED: it's on **D6**, which is a PWM pin, so it uses `analogWrite` with a
value of `greenBrightness = 10` instead of full brightness. That's about 4% duty
cycle — green LEDs are much brighter than red ones for the same current, and at
full power it washes out the other two.

Timing comes from two variables at the top:

- `longW = 1000` — how long red and green stay on
- `shortW = 500` — how long yellow stays on

## Run it

Open `Traffic_Light/Traffic_Light.ino` in the Arduino IDE, select
*Tools → Board → Arduino Uno*, pick the COM port, and click Upload.
