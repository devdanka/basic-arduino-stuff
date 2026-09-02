# 006 — Twinkle Twinkle Little Star

Plays the tune on a piezo buzzer. First project here that makes sound instead
of light.

## Parts

- Arduino Uno
- Passive piezo buzzer
- Breadboard and jumper wires

It has to be a **passive** buzzer. An active buzzer has its own oscillator
inside and only ever makes one fixed pitch, so `tone()` can't change the note.

## Wiring

| Component | Arduino pin |
|---|---|
| Buzzer + | D3 |
| Buzzer − | GND |

## How it works

`tone(pin, frequency)` starts a square wave and leaves it running; `noTone(pin)`
stops it. The note names are just frequencies in hertz, stored as variables at
the top:

| Note | Hz |
|---|---|
| Do | 261 |
| Re | 293 |
| Mi | 329 |
| Fa | 349 |
| Sol | 392 |
| La | 440 |
| Si | 493 |
| Do (octave up) | 522 |

Those are middle-C-octave frequencies. La = 440 Hz is the standard tuning pitch
every instrument tunes to, and each octave up doubles the frequency — which is
why `Do2` (522) is about twice `Do` (261).

Note lengths come from `wait = 200` ms, with `longwait = wait * 3` for the held
notes at the end of each phrase.

## Run it

Upload and it plays on a loop.
