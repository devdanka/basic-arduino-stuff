# PlatformIO projects

Fourteen Arduino Uno sketches living in one PlatformIO project. Instead of a
folder per sketch, there's a **build environment** per sketch, and each one
compiles exactly one file.

## How the one-project-many-sketches trick works

The `[env]` section in [`platformio.ini`](platformio.ini) starts by excluding
everything:

```ini
build_src_filter = -<*>
```

Then each environment adds its own single file back in:

```ini
[env:radar]
build_src_filter = ${env.build_src_filter} +<Arduino Uno/007-Radar.cpp>
```

Without that, the compiler would see fourteen `setup()` and `loop()` functions
at once and refuse to link. This way every sketch shares one library list and
one set of board settings, and `-e <env>` picks which one to build.

## Building

```bash
pio run -e radar -t upload
```

```bash
pio device monitor -e radar
```

Pass `-e <env>` to pick a sketch, or use the **PROJECT TASKS** panel in the
PlatformIO sidebar, which lists every environment with its own Build / Upload /
Monitor buttons — no need to edit `platformio.ini` as you work.

`platformio.ini` sets `default_envs = esp32`, but that project isn't part of
this repo (see the note at the bottom), so **always pass `-e <env>`** rather
than running a bare `pio run`.

## Projects

| # | Project | env | What it does |
|---|---|---|---|
| 000 | [Set LED brightness](src/Arduino%20Uno/000-SetLEDBrightness) | `led` | Two buttons ramp an LED up and down |
| 001 | [Input servo value](src/Arduino%20Uno/001-InputServoValue) | `servo-input` | Type an angle 0–180, the servo goes there |
| 002 | [Photo-detecting servo](src/Arduino%20Uno/002-PhotoDetectingServo) | `servo` | Servo angle follows the light level |
| 003 | [DHT11 thermometer](src/Arduino%20Uno/003.DHT11) | `dht11` | Temperature and humidity on an I²C LCD |
| 004 | [Keypad calculator](src/Arduino%20Uno/004-LCDI2C) | `lcd-i2c` | 4×4 keypad + LCD, does `+ − × ÷` |
| 005 | [Ultrasonic ruler](src/Arduino%20Uno/005-Ultrasonic) | `ultrasonic` | HC-SR04 distance in cm on the LCD |
| 006 | [Servo sweep](src/Arduino%20Uno/006-ServoSweep) | `servo-sweep` | Servo sweeps 0→180→0 forever |
| 007 | [**Radar**](src/Arduino%20Uno/007-Radar) | `radar` | 005 + 006 combined, streamed to a PPI display on the PC |
| 008 | [IR remote calculator](src/Arduino%20Uno/008-IRRemote) | `ir-remote` | The 004 calculator, driven by a TV remote |
| 009 | [IR servo remote](src/Arduino%20Uno/009-ServoRemote) | `servo-remote` | Remote arrows drive the servo |
| 010 | [Shift register](src/Arduino%20Uno/010-74HC595) | `74hc595` | 8 LEDs from 3 pins with a 74HC595 |
| 011 | [Relay + remote](src/Arduino%20Uno/011-RelayModule) | `relay-remote` | Remote OK button toggles a relay |
| 012 | [Relay + photoresistor](src/Arduino%20Uno/012-RelayPhotoresistor) | `relay-photoresistor` | Relay switches on when it gets dark |
| 013 | [PC triggers](src/Arduino%20Uno/013-Triggers) | `triggers` | Reports light and IR events for a PC app to turn into keystrokes |

## A note on the ESP32 environment

`platformio.ini` also carries an `[env:esp32]` section, but the sketch it
points at isn't in this repo — that project is kept private. The environment is
left in the file because it's still used locally; it just won't build from a
fresh clone.

Everything else here is Arduino Uno.

## Libraries

Declared once in `[env]` and pulled in automatically on first build:

| Library | Used by |
|---|---|
| `arduino-libraries/Servo` | 001, 002, 006, 007, 009 |
| `adafruit/DHT sensor library` + `Adafruit Unified Sensor` | 003 |
| `marcoschwartz/LiquidCrystal_I2C` | 003, 004, 005, 008 |
| `chris--a/Keypad` | 004 |
| `z3t0/IRremote` | 008, 009, 011, 013 |
| `simsso/ShiftRegister74HC595` | 010 |

## Wiring shared by several projects

The 16×2 LCD is on I²C at address **0x27**, which on an Uno is fixed to two
pins:

| LCD backpack | Arduino pin |
|---|---|
| SDA | A4 |
| SCL | A5 |
| VCC | 5V |
| GND | GND |

If the screen lights up but stays blank, try address `0x3F` instead — the two
common backpack chips (PCF8574 and PCF8574A) use different ones.
