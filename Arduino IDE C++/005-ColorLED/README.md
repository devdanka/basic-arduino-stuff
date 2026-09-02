# 005 — Color picker

Type `Red` or `Yellow` into the Serial Monitor and the matching LED blinks
once. Same idea as 004, but comparing text instead of parsing a number.

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

`Serial.readString()` grabs everything waiting in the buffer as a `String`, and
the sketch compares it directly:

```cpp
color = Serial.readString();
color.trim();
if (color == "Red")    { ... }
if (color == "Yellow") { ... }
```

Two `if`s rather than `if/else if`, so an unrecognised word simply does
nothing.

## Why `trim()` is there

This is the classic trap with reading text over serial. If the Serial Monitor's
line-ending dropdown is set to *Newline*, what actually arrives isn't `"Red"` —
it's `"Red\n"`, with the newline character still attached. And
`"Red\n" == "Red"` is **false**, so nothing lights up and the sketch looks
broken for no visible reason.

`color.trim()` strips whitespace and line endings off both ends of the string,
which makes the sketch work whatever the dropdown is set to. (The alternative
is to remember to set it to *No line ending* every time — trimming is more
reliable.)

Case still matters, though: string comparison is exact, so `red` won't match
`Red`.

## Run it

Upload, open the Serial Monitor at **9600 baud**, then type `Red` and press
Enter. Any line-ending setting works.
