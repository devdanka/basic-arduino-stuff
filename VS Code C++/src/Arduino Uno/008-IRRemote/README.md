# 008 — IR remote calculator

The same four-function calculator as [004](../004-LCDI2C),
but driven by a cheap TV-style remote instead of a keypad. Digits on the number
buttons, operators on the arrow keys, OK to calculate.

**Source:** [`008-IRRemote.cpp`](008-IRRemote.cpp)
**Build:** `pio run -e ir-remote -t upload`

## Parts

- Arduino Uno
- VS1838B IR receiver (the 3-pin black blob)
- 38 kHz IR remote — the small white/black one from an Arduino starter kit
- 16×2 LCD with I²C backpack (address 0x27)
- Breadboard and jumper wires

## Wiring

| Component | Arduino pin |
|---|---|
| IR receiver OUT | D4 |
| IR receiver VCC | 5V |
| IR receiver GND | GND |
| LCD SDA | A4 |
| LCD SCL | A5 |

The IR receiver's pinout is easy to get backwards — with the domed face
towards you, the legs are usually **OUT, GND, VCC** from left to right. Check
yours; wiring it wrong makes it get hot.

## Button codes

The sketch matches raw NEC codes. These are for the remote it was written
against:

| Code | Button |
|---|---|
| `0xBA45FF00` | 1 |
| `0xB946FF00` | 2 |
| `0xB847FF00` | 3 |
| `0xBB44FF00` | 4 |
| `0xBF40FF00` | 5 |
| `0xBC43FF00` | 6 |
| `0xF807FF00` | 7 |
| `0xEA15FF00` | 8 |
| `0xF609FF00` | 9 |
| `0xE619FF00` | 0 |
| `0xE718FF00` | ▲ — add `+` |
| `0xAD52FF00` | ▼ — subtract `−` |
| `0xA55AFF00` | ► — multiply `×` |
| `0xF708FF00` | ◄ — divide `÷` |
| `0xE31CFF00` | OK — calculate |

**If your remote is different**, the codes won't match and nothing will happen.
Find yours by uploading [013-Triggers](../013-Triggers), which prints
`IR:<hex>` for every button you press, then paste the new values in.

## How it works

An IR remote doesn't just flash a light — that would be swamped by sunlight and
room lighting. It flashes a **38 kHz carrier**, and the receiver only responds
to light blinking at that exact rate. Everything else is filtered out.

The data rides on top: the NEC protocol encodes bits as different gaps between
bursts. The library decodes all of that and hands over one 32-bit number.

The sketch builds up an expression string as you press keys, guards it at 15
characters (the LCD is 16 wide), and only allows one operator per expression —
`expression.indexOf('+') == -1 && ...` checks that none is already there.

Pressing a digit right after a result clears the display and starts fresh,
which is what the `showingResult` flag tracks.

## Run it

```bash
pio run -e ir-remote -t upload
```

Point the remote at the receiver from within a metre or so.
