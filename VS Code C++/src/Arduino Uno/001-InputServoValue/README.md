# 001 — Input servo value

Type an angle between 0 and 180 into the serial monitor and the servo goes
there. Invalid numbers get rejected with a message.

**Source:** [`001-InputServoValue.cpp`](001-InputServoValue.cpp)
**Build:** `pio run -e servo-input -t upload`

## Parts

- Arduino Uno
- SG90 micro servo (or similar hobby servo)
- Breadboard and jumper wires

## Wiring

| Servo wire | Arduino pin |
|---|---|
| Orange / yellow (signal) | D6 |
| Red (power) | 5V |
| Brown / black (ground) | GND |

One small SG90 runs fine off the Uno's 5V pin. Two or more, or anything bigger,
needs its own supply — servos yank a lot of current when they start moving, and
that dip can brown out the board and reset it. If you do use a separate supply,
its ground still has to be tied to the Arduino's ground or the signal has no
reference.

## How it works

`Servo.write(angle)` takes degrees, and the library turns that into the pulse
the servo actually wants — a pulse between roughly 1000 and 2000 µs, repeated
every 20 ms. The servo's internal circuit compares that pulse width against its
own position sensor and drives the motor until they match.

The range check happens before the write:

```cpp
if (servoPos >= 0 && servoPos <= 180) {
    myServo.write(servoPos);
} else {
    Serial.println("Invalid position...");
}
```

Worth having — pushing a servo past its mechanical stops makes it buzz and
strain against itself.

## Note on Timer1

The `Servo` library takes over Timer1, which is what generates PWM on **D9 and
D10**. `analogWrite` on those two pins stops working in any sketch that uses a
servo. They still work as ordinary digital pins — which is exactly what
[007-Radar](../007-Radar) relies on.

## Run it

```bash
pio run -e servo-input -t upload
pio device monitor -e servo-input
```

Type a number and press Enter.
