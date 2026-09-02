# 001 — Potentiometer → LED

Turn a knob, and the LED dims to match. The measured voltage is printed to the
Serial Monitor at the same time.

## Parts

- Arduino Uno
- 10 kΩ potentiometer
- LED + 220 Ω resistor
- Breadboard and jumper wires

## Wiring

| Component | Arduino pin |
|---|---|
| Potentiometer — left leg | 5V |
| Potentiometer — wiper (middle leg) | A1 |
| Potentiometer — right leg | GND |
| LED (+ 220 Ω) | D5 |
| LED cathode | GND |

D5 has to be a **PWM pin** (marked `~` on the board) because the sketch uses
`analogWrite` on it.

## How it works

`analogRead` gives a number from 0 to 1023, because the Uno's ADC is 10 bits
(2¹⁰ = 1024 possible values). The sketch converts that number twice:

```cpp
volt    = (5.  / 1023.) * result;   // 0-1023  ->  0-5 V, just for display
LEDvolt = (255./ 1023.) * result;   // 0-1023  ->  0-255, what analogWrite wants
```

The `5.` and `1023.` are written with a decimal point on purpose. Without it,
`5 / 1023` is integer division and comes out as plain `0`.

Note that `analogWrite` doesn't actually output a voltage — it switches the pin
fully on and off very fast (PWM), and the LED averages it out.

## Run it

Upload, then open *Tools → Serial Monitor* at **9600 baud** to watch the
voltage while you turn the knob.
