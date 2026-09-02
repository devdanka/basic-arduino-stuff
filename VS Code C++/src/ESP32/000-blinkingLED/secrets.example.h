#pragma once

// Template. Copy this file to secrets.h in the same folder and put your own
// network in it:
//
//     cp secrets.example.h secrets.h
//
// secrets.h is gitignored, so your real password never gets committed.
// This file, with the placeholders, is safe to commit.
//
// The ESP32 has no 5 GHz radio, so this must be a 2.4 GHz network.

#define WIFI_SSID     "YOUR_WIFI_NAME"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
