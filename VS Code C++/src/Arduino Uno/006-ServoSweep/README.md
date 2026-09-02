# 006 — Servo sweep

The servo walks from 0° to 180° and back, one degree at a time, forever. On its
own it's a test sketch; combined with [005-Ultrasonic](../005-Ultrasonic) it
becomes [007-Radar](../007-Radar).

**Source:** [`006-ServoSweep.cpp`](006-ServoSweep.cpp)
**Build:** `pio run -e servo-sweep -t upload`

## Parts

- Arduino Uno
- SG90 micro servo
- Jumper wires

## Wiring

| Servo wire | Arduino pin |
|---|---|
| Orange / yellow (signal) | A1 |
| Red (power) | 5V |
| Brown / black (ground) | GND |

**The signal is on A1, not a digital pin.** That's legal — the analog pins
double as digital pins (A1 is also D15) and the Servo library is happy to drive
any of them. `007-Radar.cpp` moves the servo to D6 instead.

## How it works

Two `for` loops, one up and one down:

```cpp
for (servoPos = 0; servoPos <= 180; servoPos += stepSize) {
    myServo.write(servoPos);
    delay(stepDelay);
}
```

`delay(stepDelay)` is doing real work here. `Servo.write()` returns
immediately — it only changes the pulse width being sent; the horn takes time
to physically get there. An SG90 needs roughly 100 ms per 60°, so about 2–3 ms
per degree. The 15 ms delay is comfortably longer than that, which keeps the
motion smooth. Drop it too low and the servo can't keep up, and the sweep turns
jerky.

Two knobs to play with:

- `stepSize = 1` — degrees per step. Larger is faster and coarser.
- `stepDelay = 15` — milliseconds between steps.

A full sweep takes 180 × 15 ms ≈ 2.7 seconds each way.

## Run it

```bash
pio run -e servo-sweep -t upload
```

No serial output — just watch it move.
