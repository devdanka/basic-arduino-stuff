# 000 — blinkingLED (ESP32 web server)

The board joins the WiFi and serves its own web page. Open it from any browser
on the network and you get a live status readout and a button that blinks the
onboard LED — the ESP32 is both the web server and the thing being controlled.

**Source:** [`000-blinkingLED.cpp`](000-blinkingLED.cpp)
**Build:** `pio run -e esp32 -t upload` **and** `pio run -e esp32 -t uploadfs`

## Parts

- ESP32 DevKit V1 (WROOM-32, 30- or 38-pin)
- USB cable
- A 2.4 GHz WiFi network

No wiring at all — it uses the onboard LED on **GPIO 2**.

## ⚠️ Set up secrets.h first

The sketch will not build until you create your credentials file. Copy the
template and fill it in:

```bash
cp secrets.example.h secrets.h
```

Then edit `secrets.h`:

```cpp
#define WIFI_SSID     "YourNetworkName"
#define WIFI_PASSWORD "YourPassword"
```

`secrets.h` is gitignored, so your real password stays on your machine and
never reaches GitHub. `secrets.example.h` holds only placeholders and is safe
to commit.

**It must be a 2.4 GHz network.** The ESP32 has no 5 GHz radio at all, so a
5 GHz-only SSID simply won't be visible to it. On most routers both bands share
one name, which works fine.

## ⚠️ Two uploads, not one

This is the thing that catches you out. The firmware and the web page live in
**different flash partitions**, written by two different commands:

```bash
pio run -e esp32 -t upload
```

```bash
pio run -e esp32 -t uploadfs
```

| Command | Sends | When you need it |
|---|---|---|
| `-t upload` | the compiled sketch | you changed anything in `src/` |
| `-t uploadfs` | everything in `data/` | you changed `data/index.html` |

Skip `uploadfs` and LittleFS is empty, so visiting `/` returns **500** and the
serial monitor prints:

```
open(): /littlefs/index.html does not exist
```

If you see that, you've only done half the upload.

## Finding the board

It's pinned to a static address so it's always at the same URL:

```
http://192.168.1.184
```

`useStaticIP = false` at the top of the sketch switches back to letting the
router assign an address, in which case the serial monitor prints whichever one
it got.

mDNS is also running, so this usually works too:

```
http://esp32.local
```

## Endpoints

| Route | Returns |
|---|---|
| `/` | `index.html`, streamed straight off LittleFS |
| `/status` | JSON — uptime, WiFi signal strength, free heap |
| `/blink` | blinks the onboard LED 5 times, replies `ok` |

The page's button calls `/blink` with `fetch()`. **This only works when the page
is served by the board.** If you open your local copy of `data/index.html` by
double-clicking it, the browser loads it from `file://`, `fetch('/blink')` has
no server to talk to, and the button does nothing at all — no error, just
silence. Always go via the board's IP address.

## What the LED is telling you

The onboard LED doubles as a status indicator, so you can tell what's happening
without opening the serial monitor:

| LED | Meaning |
|---|---|
| Fast blink (200 ms) | trying to connect |
| Slow blink (1 s) | connected and serving |
| Off | connection failed or dropped |
| Brief solid flash | a page was just served |

The sketch re-checks the connection every 10 seconds and gives up on the
initial connect after 20.

## Security

This is **LAN-only by design**: plain HTTP, no login, no encryption. Anyone on
your WiFi can open it and blink the LED, which is fine for what it does. Don't
port-forward it to the internet.

## Run it

```bash
pio run -e esp32 -t upload
```

```bash
pio run -e esp32 -t uploadfs
```

```bash
pio device monitor -e esp32
```

The monitor runs at **115200 baud** here, not 9600 like the Uno sketches — it
prints the IP address once connected. `monitor_filters = esp32_exception_decoder`
in `platformio.ini` turns crash addresses into file names and line numbers,
which makes debugging far less painful.
