# 007 — Radar

An ultrasonic sensor riding on a sweeping servo. Every reading has a **bearing**
as well as a **range**, which is everything a radar screen needs — the readings
stream out over serial and a companion PC app draws them on a PPI scope (the
round green sweep you see in films).

This is [005-Ultrasonic](../005-Ultrasonic) (the distance reading) bolted onto
[006-ServoSweep](../006-ServoSweep) (the motion).

**Source:** [`007-Radar.cpp`](007-Radar.cpp)
**Build:** `pio run -e radar -t upload`

## Parts

- Arduino Uno
- HC-SR04 ultrasonic sensor
- SG90 micro servo
- Something to mount the sensor on the servo horn (tape works)
- Jumper wires

## Wiring

| Component | Arduino pin |
|---|---|
| HC-SR04 Trig | D10 |
| HC-SR04 Echo | D9 |
| HC-SR04 VCC / GND | 5V / GND |
| Servo signal (orange) | D6 |
| Servo power (red) | 5V |
| Servo ground (brown) | GND |

Mount the sensor on the servo horn facing **away from the shaft**, so it looks
wherever the horn points.

### Why the servo isn't on D9 or D10

The Servo library takes over Timer1, and Timer1 is what generates PWM on D9 and
D10. So `analogWrite` on those two stops working — but D9 and D10 are used here
as **plain digital pins** for trig and echo, which Timer1 doesn't affect at all.
The servo gets D6 instead. No conflict.

## Serial protocol

9600 baud, one line per reading:

```
90,42.3
```

| Field | Meaning |
|---|---|
| `90` | bearing in degrees |
| `42.3` | range in centimetres, or `-1.0` if nothing echoed back |

Lines starting with `#` are comments and are ignored by the app — the sketch
sends `# radar ready` once on boot.

That's it. Plain text, one reading per line, no framing or checksums — anything
that can open a serial port can read it, including the Serial Monitor.

## Design decisions

**Angles stop at 15° and 165°, not 0° and 180°.** Hobby servos are unreliable
in the last few degrees of travel — they buzz, stall, or overshoot. Giving up
15° at each end costs nothing and makes the sweep clean.

**25 ms settle time before each ping.** `Servo.write()` returns straight away,
but the horn takes time to actually arrive. Pinging too early measures the
distance to wherever the sensor *was*, and the picture smears.

**A 25 ms echo timeout.** Sound covers 1 cm every 29.1 µs each way, so 25 ms is
about 4.3 m — past the HC-SR04's honest range. Without it, `pulseIn` blocks for
a full second every time nothing echoes back, and one missing return would stall
the whole sweep.

**No LCD.** An I²C write between every step would stall the sweep and make the
display jerky. The PC screen *is* the display now.

## Sweep rate

150° of travel at 1° per step, 25 ms of settle plus up to 25 ms of ping =
roughly 4–7 seconds per sweep. Raise `STEP` to 2 or 3 to trade angular
resolution for a faster refresh.

## Run it

```bash
pio run -e radar -t upload
pio device monitor -e radar
```

You should see a stream of `angle,distance` lines. Close the monitor before
starting the PC display app — only one program can hold a COM port at a time.

The PC-side display isn't part of this repo; it's a separate project that reads
this serial stream and plots it.
