# 007 — Frequency change

Turns a potentiometer into a pitch knob. Turn it and the buzzer slides from
silence up to about 10 kHz.

## Parts

- Arduino Uno
- 10 kΩ potentiometer
- Passive piezo buzzer
- Breadboard and jumper wires

## Wiring

| Component | Arduino pin |
|---|---|
| Potentiometer — top leg | D11 (used as a 5 V rail) |
| Potentiometer — wiper (middle leg) | A0 |
| Potentiometer — bottom leg | GND |
| Buzzer + | D3 |
| Buzzer − | GND |

D11 is driven `HIGH` in `loop()` and used to power the potentiometer instead of
running a wire to the 5V pin. It works — an output pin can source up to 40 mA
and a 10 kΩ pot draws well under 1 mA — but wiring the pot to **5V** is the
normal way to do it and frees up a pin.

## How it works

```cpp
note = (9940. / 1023.) * read;
```

That maps the 0–1023 reading onto a frequency range topping out at 9940 Hz.

**The decimal points are load-bearing.** Written as `(9940 / 1023)` — two whole
numbers — C++ does integer division and throws away the remainder, giving
exactly **9**. The formula would silently become `note = 9 * read`, and the top
of the range would be 9207 Hz instead of 9940. The dots make them `double`s, so
the division keeps its fractional part (9.7166…) and only gets rounded once, at
the end, when the result is stored into the `int`.

Same trick as the `volt` line above it. Integer division catching you out is
one of the most common bugs in Arduino code, and it never announces itself —
the sketch just quietly computes the wrong number.

The `volt` line is display only — it converts the same reading into volts for
the Serial Monitor.

## Run it

Upload, open the Serial Monitor at **9600 baud**, and turn the knob.
