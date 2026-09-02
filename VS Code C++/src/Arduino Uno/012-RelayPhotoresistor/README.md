# 012 — Relay + photoresistor

An automatic night light. When the photoresistor sees the room get dark, the
relay switches on; when it brightens again, the relay switches off.

**Source:** [`012-RelayPhotoresistor.cpp`](012-RelayPhotoresistor.cpp)
**Build:** `pio run -e relay-photoresistor -t upload`

## ⚠️ Mains voltage

Same warning as [011](../011-RelayModule): switch low-voltage DC loads only.
Household mains on a breadboard is genuinely dangerous.

## Parts

- Arduino Uno
- 1-channel 5 V relay module
- Photoresistor (LDR)
- 10 kΩ resistor
- A low-voltage load to switch
- Breadboard and jumper wires

## Wiring

| Component | Arduino pin |
|---|---|
| Relay IN | D6 |
| Relay VCC / GND | 5V / GND |
| LDR — one leg | D5 (used as a 5 V rail) |
| LDR — other leg | A0 **and** one leg of the 10 kΩ resistor |
| 10 kΩ resistor — other leg | GND |

```
D5 (HIGH) ---[ LDR ]---+---[ 10k ]--- GND
                       |
                      A0
```

D5 is driven HIGH in `setup()` and used to power the divider. It works fine
here (a 10 kΩ divider draws well under a milliamp), and it means you could
switch the sensor off in software if you ever wanted to.

## Hysteresis — the good bit

This is the important idea in the sketch. A single threshold would make the
relay chatter: right at the boundary, noise pushes the reading one count either
side and the relay clicks on and off many times a second, which wears out the
contacts fast.

The fix is two thresholds instead of one — a `margin` of 20 counts around the
setpoint of 200:

```cpp
if (!relayState && photoresistorValue < threshold - margin) {   // < 180: switch ON
  relayState = true;
  digitalWrite(pinRelay, LOW);
} else if (relayState && photoresistorValue > threshold + margin) {  // > 220: switch OFF
  relayState = false;
  digitalWrite(pinRelay, HIGH);
}
```

It has to get properly dark (below 180) to switch on, and properly bright
(above 220) to switch off again. In between, whatever the relay is already
doing, it keeps doing. Checking `!relayState` and `relayState` first means the
pin only gets written when the state actually changes.

This is the same trick as a thermostat that won't fire the boiler until the
room drops a degree below the target. Nearly every real sensor threshold needs
it.

## Calibrating

`threshold = 200` is specific to one LDR in one room. Run the monitor first and
see what your setup reads:

```bash
pio device monitor -e relay-photoresistor
```

It prints the raw value and the relay state on every pass. Cover the LDR, note
the number; shine a light on it, note that one; put the midpoint into
`threshold`.

Remember the relay is **active low** — `digitalWrite(pinRelay, LOW)` turns it
on.

## Run it

```bash
pio run -e relay-photoresistor -t upload
```
