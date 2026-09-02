# 004 — Serial input

The board asks a question in the Serial Monitor — *"How many times do you want
the red LED to blink?"* — waits for you to type a number, then blinks that many
times.

## Parts

- Arduino Uno
- Red LED + 220 Ω resistor
- Breadboard and jumper wires

## Wiring

| Component | Arduino pin |
|---|---|
| Red LED (+ 220 Ω) | D10 |
| LED cathode | GND |

## How it works

Two pieces do the work. First, the sketch parks itself until something arrives:

```cpp
while (Serial.available() == 0) {
}
```

`Serial.available()` returns how many bytes are sitting in the receive buffer,
so an empty `while` body just spins until it's non-zero. Then
`Serial.parseInt()` pulls the digits out and turns them into an `int`, and a
`for` loop blinks that many times.

## Run it

Upload, open the Serial Monitor at **9600 baud**, type a number in the input
box at the top, and press Enter.

If nothing happens, check the line-ending dropdown at the bottom of the Serial
Monitor — `parseInt()` copes with any setting, but the sketches in this folder
that read *text* need it set to **No line ending**.
