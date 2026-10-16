# Serial input triggers

Watches a photoresistor and an IR receiver, sending `DARK`, `LIGHT`, or `IR:<hex>` events over serial at 9600 baud. Light events appear only when the state changes, and repeated IR frames are ignored. A separate PC program can turn these events into actions such as key presses.
