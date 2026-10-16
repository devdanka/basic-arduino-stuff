// ---------------------------------------------------------------------------
// 001-OTA - the smallest sketch that makes wireless uploading possible.
//
// This does nothing else. It joins WiFi, opens the OTA listener, and blinks.
// Flash it once over the USB cable; after that the board shows up as a network
// port and every later upload can go over the air.
//
// Read the README.md next to this file for the two commands.
// ---------------------------------------------------------------------------

#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>

// Same arrangement as 000-blinkingLED: the network lives in secrets.h, which is
// gitignored. Copy secrets.example.h to secrets.h and fill it in.
#include "secrets.h"

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

// The name this board answers to on the network. It is what appears in the
// Arduino IDE's Port menu under "Network ports", and it is what you put in
// upload_port in platformio.ini. Deliberately different from the web server
// sketch's "esp32" so you can tell the two apart if both are ever running.
const char* otaHostname = "esp32-ota";

// A fixed address, matching 000-blinkingLED, so the upload target never moves.
// Set useStaticIP to false to go back to letting the router decide - but then
// you have to read the address off the serial monitor after every reboot.
const bool useStaticIP = true;
IPAddress staticIP(192, 168, 1, 184);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dnsServer(192, 168, 1, 1);

const int ledPin = 2;                  // onboard blue LED on most DevKit boards
const unsigned long timeout = 20000;   // stop trying to connect after 20 s
const unsigned long checkEvery = 10000;

// What the LED is saying right now. Since this sketch has no other output, the
// LED is how you know from across the room whether OTA is actually listening.
//   connecting -> fast blink       listening -> one short wink every 2 s
//   offline    -> slow blink       uploading -> brightness tracks progress
enum LedState { CONNECTING, LISTENING, OFFLINE };
LedState ledState = CONNECTING;

unsigned long lastCheck = 0;
unsigned long lastToggle = 0;
bool ledOn = false;
bool otaInProgress = false;

// Non-blocking. Nothing in this sketch is allowed to sit in a delay(), because
// ArduinoOTA.handle() has to run constantly or an upload will stall and fail.
void updateLed() {
  if (otaInProgress) {
    return;                     // onProgress() drives the LED during an upload
  }

  if (ledState == LISTENING) {
    // Heartbeat: on for 60 ms out of every 2 s. Idle but alive.
    unsigned long phase = millis() % 2000;
    digitalWrite(ledPin, phase < 60 ? HIGH : LOW);
    return;
  }

  unsigned long interval = (ledState == CONNECTING) ? 200 : 1000;
  if (millis() - lastToggle >= interval) {
    lastToggle = millis();
    ledOn = !ledOn;
    digitalWrite(ledPin, ledOn);
  }
}

bool connectToWiFi() {
  // Without this you just get a silent 20 s timeout and no hint as to why.
  if (String(ssid) == "YOUR_WIFI_NAME") {
    ledState = OFFLINE;
    Serial.println("secrets.h still has the placeholder network. Fill it in, "
                   "then upload again over USB.");
    return false;
  }

  ledState = CONNECTING;
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);          // join a network, don't become one

  // Must come before begin(). If it fails the board still joins, just on DHCP.
  if (useStaticIP && !WiFi.config(staticIP, gateway, subnet, dnsServer)) {
    Serial.println("Static IP rejected, falling back to DHCP.");
  }

  WiFi.begin(ssid, password);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < timeout) {
    updateLed();                // blink while we wait
    delay(10);
  }

  if (WiFi.status() != WL_CONNECTED) {
    ledState = OFFLINE;
    Serial.println("Failed. Check the name, the password, and that it is 2.4 GHz.");
    Serial.println("No WiFi means no OTA - reflash over USB once it is fixed.");
    return false;
  }

  ledState = LISTENING;
  Serial.print("Connected, IP is ");
  Serial.println(WiFi.localIP());
  return true;
}

void startOTA() {
  ArduinoOTA.setHostname(otaHostname);

  // Without a password anyone on your WiFi can overwrite the firmware. The
  // uploader has to send the same one, which platformio.ini does via
  // upload_flags = --auth=...
  ArduinoOTA.setPassword(OTA_PASSWORD);

  ArduinoOTA.onStart([]() {
    otaInProgress = true;
    digitalWrite(ledPin, LOW);
    // U_FLASH is the program itself; the other command is the LittleFS image,
    // which arrives when you run `-t uploadfs` instead of `-t upload`.
    Serial.println(ArduinoOTA.getCommand() == U_FLASH
                       ? "OTA starting: firmware"
                       : "OTA starting: filesystem");
  });

  ArduinoOTA.onProgress([](unsigned int done, unsigned int total) {
    // Twenty flickers spread across the transfer, so you can see it moving.
    // Deliberately digitalWrite and not analogWrite: on this core analogWrite
    // hands the pin to the LEDC peripheral and never gives it back, so a
    // failed upload would leave the LED frozen at whatever level it reached.
    unsigned int step = total / 20 + 1;
    digitalWrite(ledPin, (done / step) % 2);
    Serial.printf("  %u%%\r", total ? (done * 100U) / total : 0);
  });

  ArduinoOTA.onEnd([]() {
    otaInProgress = false;
    digitalWrite(ledPin, HIGH);   // solid on for the moment before it reboots
    Serial.println("\nOTA done, rebooting into the new firmware.");
  });

  ArduinoOTA.onError([](ota_error_t error) {
    otaInProgress = false;
    Serial.printf("\nOTA failed (%u): ", error);
    switch (error) {
      case OTA_AUTH_ERROR:    Serial.println("wrong password"); break;
      case OTA_BEGIN_ERROR:   Serial.println("no room - the new binary is "
                                             "bigger than the spare OTA slot"); break;
      case OTA_CONNECT_ERROR: Serial.println("could not reach the board"); break;
      case OTA_RECEIVE_ERROR: Serial.println("transfer cut off partway"); break;
      case OTA_END_ERROR:     Serial.println("flash write did not verify"); break;
      default:                Serial.println("unknown"); break;
    }
  });

  // This also starts mDNS and advertises _arduino._tcp on port 3232, which is
  // what makes the board appear under "Network ports" in the Arduino IDE.
  ArduinoOTA.begin();

  Serial.println("OTA listening.");
  Serial.print("  upload_port = ");
  Serial.print(WiFi.localIP());
  Serial.print("   (or ");
  Serial.print(otaHostname);
  Serial.println(".local)");
}

void setup() {
  pinMode(ledPin, OUTPUT);
  Serial.begin(115200);
  delay(1000);                   // let the monitor open after reset

  Serial.println();
  Serial.println("001-OTA - wireless upload listener, nothing else.");

  if (connectToWiFi()) {
    startOTA();
  }
}

void loop() {
  // Has to run on every pass. Anything that blocks here - a delay(), a long
  // Serial print loop, a busy sensor read - will make uploads time out.
  ArduinoOTA.handle();
  updateLed();

  if (millis() - lastCheck < checkEvery) {
    return;
  }
  lastCheck = millis();

  if (WiFi.status() == WL_CONNECTED) {
    if (ledState != LISTENING) {
      // Came back after a drop. ArduinoOTA is already begun, but mDNS needs
      // the fresh connection, so restart the listener rather than guess.
      ledState = LISTENING;
      startOTA();
    }
  } else {
    ledState = OFFLINE;
    Serial.println("Dropped, reconnecting...");
    if (connectToWiFi()) {
      startOTA();
    }
  }
}
