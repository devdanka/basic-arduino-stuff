# 003 — DHT11 thermometer

Temperature and humidity, read from a DHT11 and shown on a 16×2 I²C LCD,
refreshed every 2 seconds.

**Source:** [`003.DHT11.cpp`](003.DHT11.cpp)
**Build:** `pio run -e dht11 -t upload`

## Parts

- Arduino Uno
- DHT11 temperature/humidity sensor
- 16×2 LCD with an I²C backpack (address 0x27)
- Breadboard and jumper wires

Bare DHT11 modules need a 10 kΩ pull-up from the data pin to 5V. Most breakout
boards (the little blue ones with 3 pins) already have it on board.

## Wiring

| Component | Arduino pin |
|---|---|
| DHT11 data | D13 |
| DHT11 VCC / GND | 5V / GND |
| LCD SDA | A4 |
| LCD SCL | A5 |
| LCD VCC / GND | 5V / GND |

A4 and A5 aren't a choice — they're the Uno's hardware I²C pins, and every I²C
device has to go on those two. The upside is that all I²C devices share them,
so you can hang several off the same pair as long as they have different
addresses.

## How it works

The DHT11 doesn't use I²C or SPI. It has one wire and a timing-based protocol
of its own, which the library handles:

```cpp
DHT dht(13, DHT11);
float h = dht.readHumidity();
float t = dht.readTemperature();
```

**The `isnan` check is important.** A failed read returns "not a number" rather
than an error code, and `nan` propagates silently through arithmetic — so
without this guard the LCD would show garbage:

```cpp
if (isnan(t) || isnan(h)) {
  lcd.print("Sensor error");
  return;
}
```

`(char)223` is the degree symbol in the LCD's built-in character ROM. That
table isn't ASCII, so writing `"°"` directly gives you the wrong glyph.

The 2-second delay isn't just for readability — a DHT11 only samples once per
second and returns a cached value if you ask faster.

## Accuracy

DHT11 is the cheap one: ±2 °C and ±5 % RH, whole numbers only. The DHT22 is
pin-compatible and much better (±0.5 °C, one decimal) if the readings matter.

## Run it

```bash
pio run -e dht11 -t upload
```

Nothing prints to serial — the LCD is the only output.
