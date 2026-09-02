# 009 — IR servo remote

Drive the servo with a TV remote. Press ► and it starts turning one way; press
it again and it stops. Press ◄ and it goes the other way.

**Source:** [`009-ServoRemote.cpp`](009-ServoRemote.cpp)
**Build:** `pio run -e servo-remote -t upload`

## Parts

- Arduino Uno
- VS1838B IR receiver
- 38 kHz IR remote
- SG90 micro servo
- Breadboard and jumper wires

## Wiring

| Component | Arduino pin |
|---|---|
| IR receiver OUT | D4 |
| IR receiver VCC / GND | 5V / GND |
| Servo signal (orange) | D6 |
| Servo power (red) | 5V |
| Servo ground (brown) | GND |

## Buttons

| Code | Button | Effect |
|---|---|---|
| `0xA55AFF00` | ► | toggle "moving right" on/off |
| `0xF708FF00` | ◄ | toggle "moving left" on/off |

Every other button is ignored.

## How it works

The interesting part is that this sketch never blocks. It's built around
`millis()` instead of `delay()`, so it can watch for remote presses and move
the servo at the same time.

**Latched direction, not hold-to-move.** IR remotes send a repeat frame roughly
every 110 ms while a button is held, which is far too coarse for smooth motion.
So a press sets a direction and the servo keeps going until you press again:

```cpp
heldDirection = heldDirection == 1 ? 0 : 1;   // pressing ► again stops it
```

**Two separate timers.**

- `MOVE_INTERVAL = 60` ms — how often the servo steps, 5° at a time.
- `PRESS_DEBOUNCE = 350` ms — how long before the *same* button counts as a new
  press rather than a repeat frame.

Without the debounce, holding the button down would toggle the direction ten
times a second and the servo would twitch in place.

The `?:` in that line is a **ternary operator** — shorthand for
`if (heldDirection == 1) heldDirection = 0; else heldDirection = 1;`.

## If your remote is different

The codes above are for one specific remote. Upload
[013-Triggers](../013-Triggers) to print the hex code of every button you
press, then substitute yours.

## Run it

```bash
pio run -e servo-remote -t upload
pio device monitor -e servo-remote
```

The monitor prints `IR servo remote ready` on boot and the current direction on
every press.
