# 011 — Relay + remote

The OK button on a TV remote toggles a relay. The relay is a mechanical switch,
so this is the project that lets a 5 V Arduino control something it has no
business touching directly.

**Source:** [`011-RelayModule.cpp`](011-RelayModule.cpp)
**Build:** `pio run -e relay-remote -t upload`

## ⚠️ Mains voltage

A relay module is happy switching a battery-powered lamp, an LED strip, or a
motor. **Do not wire household mains through it.** 240 V will kill you, the
screw terminals are exposed, and a breadboard is nowhere near safe enough for
it. Practise on low-voltage DC — the wiring and the code are identical.

## Parts

- Arduino Uno
- 1-channel 5 V relay module (the blue cube on a small board)
- VS1838B IR receiver
- 38 kHz IR remote
- A low-voltage load to switch — a 5 V or 12 V LED strip, a small DC motor,
  a battery-powered lamp
- Jumper wires

## Wiring

| Component | Arduino pin |
|---|---|
| Relay IN | D6 |
| Relay VCC | 5V |
| Relay GND | GND |
| IR receiver OUT | D12 |
| IR receiver VCC / GND | 5V / GND |

Load side: the relay has three screw terminals — **COM** (common), **NO**
(normally open) and **NC** (normally closed). Wire your load between COM and NO
and it stays off until the relay clicks on.

## Active low

The relay module triggers on **LOW**, not HIGH. That's why `setup()` starts by
writing HIGH:

```cpp
digitalWrite(pinRelay, HIGH);   // HIGH = relay OFF
```

Without that line the relay would click on the instant the board powered up,
before the sketch had decided anything. Most cheap modules are wired this way
(there's an opto-isolator inside pulling the trigger the other way round), and
it catches everybody out at least once.

## How it works

```cpp
if (IrReceiver.decodedIRData.decodedRawData == 0xE31CFF00 ||
    IrReceiver.decodedIRData.command == 0x34) {
  relayState = !relayState;
  digitalWrite(pinRelay, relayState ? LOW : HIGH);
}
```

`0xE31CFF00` is the OK button. The second check on `command == 0x34` is a
fallback — the NEC protocol splits its 32 bits into an address and a command,
and matching just the command makes the sketch a bit more tolerant of remote
variations.

`IrReceiver.resume()` at the end of the block re-arms the receiver for the next
code. Forget it and the sketch decodes one button press and then goes deaf.

## Note on the include

This sketch uses `#include <IRremote.h>` (the old header) while
[008](../008-IRRemote), [009](../009-ServoRemote) and
[013](../013-Triggers) use `IRremote.hpp`. Version 4 of the library wants the
`.hpp` form; the `.h` one still compiles but warns.

## Run it

```bash
pio run -e relay-remote -t upload
```

Press OK on the remote. You'll hear the relay click.
