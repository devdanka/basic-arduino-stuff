# 013 — PC triggers

Turns the Arduino into an input device for the PC. It watches a photoresistor
and an IR receiver, and prints a short named event over serial whenever
something happens. A Windows app on the other end maps those names to
keystrokes.

```
Arduino (this sketch)  --serial 9600-->  Trigger.exe  --SendInput-->  Windows
     "DARK" / "IR:E31CFF00"                triggers.txt maps them
```

**Source:** [`013-Triggers.cpp`](013-Triggers.cpp)
**Build:** `pio run -e triggers -t upload`

## Parts

- Arduino Uno
- Photoresistor (LDR)
- 10 kΩ resistor
- VS1838B IR receiver
- 38 kHz IR remote
- Breadboard and jumper wires

## Wiring

Unchanged from [011](../011-RelayModule) and
[012](../012-RelayPhotoresistor), minus the relay:

| Component | Arduino pin |
|---|---|
| LDR — one leg | D5 (used as a 5 V rail) |
| LDR — other leg | A0 **and** one leg of the 10 kΩ resistor |
| 10 kΩ resistor — other leg | GND |
| IR receiver OUT | D12 |
| IR receiver VCC / GND | 5V / GND |

```
D5 (HIGH) ---[ LDR ]---+---[ 10k ]--- GND        IR receiver OUT -> D12
                       |
                      A0
```

The relay is deliberately not here — 011 and 012 still drive that. This sketch
only *reports*; the PC decides what an event means.

## Output format

9600 baud, one event per line:

| Line | Meaning |
|---|---|
| `DARK` | the LDR just became covered |
| `LIGHT` | the LDR just became uncovered |
| `IR:E31CFF00` | that remote button was just pressed (raw NEC code, hex) |

## Edges, not levels

This is the design idea worth taking away. The PC wants to know *"it just went
dark"*, not *"it is dark"*. Reporting the level would send a few hundred lines a
second, and mapped to a keystroke that would jam the key down.

So the sketch tracks a `covered` flag and only prints when it flips:

```cpp
if (!covered && photoresistorValue < threshold - margin) {
  covered = true;
  Serial.println("DARK");
} else if (covered && photoresistorValue > threshold + margin) {
  covered = false;
  Serial.println("LIGHT");
}
```

Same hysteresis as 012 — a 20-count margin around a threshold of 500 stops it
from stuttering at the boundary.

The IR side does the same thing by filtering out repeat frames, which a remote
sends about every 110 ms while a button is held:

```cpp
bool isRepeat = IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT;
if (!isRepeat && IrReceiver.decodedIRData.decodedRawData != 0) { ... }
```

## No delay()

There's deliberately no `delay()` in `loop()`. The IR receiver needs a
responsive loop to catch codes, and since nothing prints unless something
actually happened, a fast loop costs nothing.

## Using it to find remote codes

This sketch is the easiest way to read the hex codes off any remote — every
button press prints its code. That's how the tables in
[008](../008-IRRemote) and [009](../009-ServoRemote) were made.

## Run it

```bash
pio run -e triggers -t upload
pio device monitor -e triggers
```

Cover the LDR and press buttons on the remote to watch the events appear. Close
the monitor before starting the PC-side app — only one program can hold a COM
port at a time.

The PC-side app isn't part of this repo.
