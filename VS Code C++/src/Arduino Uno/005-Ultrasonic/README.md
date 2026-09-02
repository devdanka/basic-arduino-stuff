# 005 — Ultrasonic ruler

Measures the distance to whatever is in front of the sensor and shows it in
centimetres on the LCD, twice a second.

**Source:** [`005-Ultrasonic.cpp`](005-Ultrasonic.cpp)
**Build:** `pio run -e ultrasonic -t upload`

## Parts

- Arduino Uno
- HC-SR04 ultrasonic sensor
- 16×2 LCD with I²C backpack (address 0x27)
- Breadboard and jumper wires

## Wiring

| Component | Arduino pin |
|---|---|
| HC-SR04 Trig | D10 |
| HC-SR04 Echo | D9 |
| HC-SR04 VCC | 5V |
| HC-SR04 GND | GND |
| LCD SDA | A4 |
| LCD SCL | A5 |

## How it works

The sensor is a tiny sonar. It chirps at 40 kHz — well above human hearing —
and times how long the echo takes to come back.

**Sending the ping.** The datasheet asks for a 10 µs pulse on Trig, and the
2 µs LOW first makes sure the line starts clean:

```cpp
digitalWrite(trigPin, LOW);
delayMicroseconds(2);
digitalWrite(trigPin, HIGH);
delayMicroseconds(10);
digitalWrite(trigPin, LOW);
```

**Timing the echo.** The sensor then holds Echo HIGH for exactly as long as the
sound is in flight, and `pulseIn` measures that in microseconds.

**The maths.**

```cpp
distance = (duration / 2) / 29.1;
```

Two steps, and both matter:

- **`/ 2`** — the sound travels to the object *and back*, so the one-way
  distance is half the round trip.
- **`/ 29.1`** — sound moves at about 343 m/s at room temperature, which works
  out to 1 cm every 29.1 microseconds.

Sound gets faster as air gets warmer (roughly +0.6 m/s per °C), so a reading
taken on a hot day is slightly off unless you correct for it.

## Limitations

- Useful range is about 2 cm to 4 m.
- Soft things — cloth, foam, fur — absorb the chirp and read as nothing.
- Angled surfaces bounce the echo away instead of back. The sensor wants a flat
  face roughly perpendicular to it.
- With no echo, `pulseIn` blocks for a full second by default before giving up.
  [007-Radar](../007-Radar) fixes that with a timeout.

## Run it

```bash
pio run -e ultrasonic -t upload
```

Output goes to the LCD only.
