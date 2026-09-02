# 009 — Toggle button

One press turns the LED on, the next press turns it off. The trick is reacting
to the *moment* the button is pressed rather than to it being held down.

## Parts

- Arduino Uno
- Push button (tactile switch)
- 10 kΩ resistor (pull-down)
- Red LED + 220 Ω resistor
- Breadboard and jumper wires

## Wiring

| Component | Arduino pin |
|---|---|
| Button — one side | D11 (used as a 5 V rail) |
| Button — other side | D3 **and** one leg of the 10 kΩ resistor |
| 10 kΩ resistor — other leg | GND |
| Red LED (+ 220 Ω) | D6 |
| LED cathode | GND |

```
D11 (HIGH) ---[ button ]---+---[ 10k ]--- GND
                           |
                          D3
```

The 10 kΩ **pull-down** matters. Without it, D3 isn't connected to anything
while the button is open — it floats, picks up electrical noise, and reads
random 1s and 0s. The resistor ties it to GND so an unpressed button reads a
solid `0`.

## How it works

The sketch remembers the previous reading and only acts on the transition from
0 to 1:

```cpp
newL = digitalRead(readPin);
if (old == 0 && newL == 1) {   // rising edge - it JUST got pressed
  ...flip the LED...
}
old = newL;
```

This is **edge detection**. Without it, holding the button down would flip the
LED on every pass of `loop()` — hundreds of times a second — and the LED would
just look dim instead of toggling.

`delay(wait)` at 100 ms doubles as crude **debouncing**: the metal contacts
inside a button bounce for a millisecond or two when pressed, which would
otherwise register as several presses.

## Run it

Upload, then press the button. Open the Serial Monitor at **9600 baud** to
watch the raw pin reading if it misbehaves.
