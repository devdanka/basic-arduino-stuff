# 008 — Light alarm

A photoresistor watches for bright light. When someone shines a torch on it,
the buzzer beeps and a red LED flashes along with it.

## Parts

- Arduino Uno
- Photoresistor (LDR)
- 10 kΩ resistor (for the voltage divider)
- Passive piezo buzzer
- Red LED + 220 Ω resistor
- Breadboard and jumper wires

## Wiring

| Component | Arduino pin |
|---|---|
| LDR — one leg | D11 (used as a 5 V rail) |
| LDR — other leg | A0 **and** one leg of the 10 kΩ resistor |
| 10 kΩ resistor — other leg | GND |
| Buzzer + | D3 |
| Buzzer − | GND |
| Red LED (+ 220 Ω) | D6 |
| LED cathode | GND |

```
D11 (HIGH) ---[ LDR ]---+---[ 10k ]--- GND
                        |
                       A0
```

An LDR on its own can't be read by an analog pin — a pin measures *voltage*,
not resistance. Pairing it with a fixed resistor makes a **voltage divider**, and
the junction between them moves up and down as the LDR's resistance changes.
More light → lower LDR resistance → higher voltage at A0.

## How it works

```cpp
if (read > 1000) {
  tone(buzzPin, 1000);
  digitalWrite(redPin, HIGH);
  ...
}
```

The alarm fires above **1000**, and `analogRead` maxes out at 1023 — so this
only triggers under genuinely bright light. If it never goes off, watch the
Serial Monitor to see what your room actually reads and lower the threshold to
suit.

The `1000` inside `tone()` is unrelated — that's the beep frequency in hertz.

The empty `if (read < 1000) { }` block does nothing; it's a leftover.

## Run it

Upload, open the Serial Monitor at **9600 baud**, and shine a phone torch at
the LDR.
