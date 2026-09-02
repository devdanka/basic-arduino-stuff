# 003 — While loops

The LED stays on for as long as an analog reading sits below 800, and switches
off the moment it climbs above. Unlike a `for` loop, nobody knows in advance
how many times this will repeat — that's what makes it a `while` loop.

## Parts

- Arduino Uno
- 10 kΩ potentiometer (or a photoresistor + 10 kΩ resistor as a divider)
- LED + 220 Ω resistor
- Breadboard and jumper wires

## Wiring

| Component | Arduino pin |
|---|---|
| Potentiometer — left leg | 5V |
| Potentiometer — wiper (middle leg) | A1 |
| Potentiometer — right leg | GND |
| LED (+ 220 Ω) | D10 |
| LED cathode | GND |

## How it works

The important detail is *inside* the loop:

```cpp
while (read < 800) {
  digitalWrite(redPin, HIGH);
  read = analogRead(readPin);   // <- re-read, or this never ends
  Serial.println(read);
  delay(wait);
}
```

The reading has to be taken again on every pass. If `read` were only set once
before the loop, the condition would never change and the sketch would hang
there forever. Forgetting this is the classic way to write an infinite loop.

`800` out of 1023 is roughly 3.9 V.

## Run it

Upload, open the Serial Monitor at **9600 baud**, and turn the knob past the
threshold to watch the LED switch.
