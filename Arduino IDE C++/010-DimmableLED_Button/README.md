# 010 — Dimmable LED

Hold the button and the LED fades up. Let go and it fades back down. A soft
dimmer instead of an on/off switch.

## Parts

- Arduino Uno
- Push button (tactile switch)
- 10 kΩ resistor (pull-up)
- Red LED + 220 Ω resistor
- Breadboard and jumper wires

## Wiring

| Component | Arduino pin |
|---|---|
| Button — one side | GND |
| Button — other side | A0 **and** one leg of the 10 kΩ resistor |
| 10 kΩ resistor — other leg | 5V |
| Red LED (+ 220 Ω) | D3 |
| LED cathode | GND |

The sketch treats a press as `readUp == 0`, so the button is wired **active
low**: pulled up to 5 V normally, shorted to GND when pressed.

D3 must be a PWM pin (`~` on the board) — that's what makes the fade possible.

## How it works

Two things worth noticing.

**A0 is read with `digitalRead`, not `analogRead`.** The analog pins double as
ordinary digital pins (A0 is also known as D14), so this is legal — the sketch
only cares whether the button is pressed, not by how much.

**The brightness ramps instead of jumping.** Every pass of `loop()` adds or
subtracts 1, and `delay(wait)` is only 10 ms:

```cpp
if (readUp == 0)  brightness = brightness + 1;
else              brightness = brightness - 1;
analogWrite(redPin, brightness);
```

255 steps × 10 ms ≈ 2.5 seconds from off to full. The `if (brightness >= 254)`
and `if (brightness <= 1)` guards clamp it at the ends — without them the value
would run past 255, wrap around, and the LED would snap from full brightness
back to off.

## Run it

Upload, then hold the button. Open the Serial Monitor at **9600 baud** to watch
the brightness value climb and fall.
