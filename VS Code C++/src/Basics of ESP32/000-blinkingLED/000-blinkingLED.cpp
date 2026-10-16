#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <LittleFS.h>
#include <ArduinoOTA.h>
#include <BluetoothSerial.h>

// Your network lives in secrets.h, which is gitignored so the password never
// reaches GitHub. Copy secrets.example.h to secrets.h and fill it in.
#include "secrets.h"

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

// A fixed address on your own LAN, so the board is always at the same URL
// instead of wherever the router feels like putting it. Your gateway is
// 192.168.1.1 and your laptop sits at 192.168.1.3, so .184 is well clear of
// both. Routers hand out DHCP leases from the bottom of the range up, so a
// high number is unlikely to collide - if it ever does, change it here.
// Set useStaticIP to false to go back to letting the router decide.
const bool useStaticIP = true;
IPAddress staticIP(192, 168, 1, 184);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dnsServer(192, 168, 1, 1);

const int ledPin = 2;                  // onboard blue LED on most DevKit boards
const unsigned long timeout = 20000;   // stop trying to connect after 20 s
const unsigned long checkEvery = 10000;

WebServer server(80);

// Bluetooth is a second serial monitor, not a second upload route - the ESP32
// bootloader only ever accepts firmware over UART, so nothing can be flashed
// through here. Pair the board in Windows Bluetooth settings, take the
// *Outgoing* COM port Windows creates, and put it in monitor_port. Uploads
// keep going over WiFi via the esp32-air env.
BluetoothSerial SerialBT;
bool btReady = false;

// Fans every log line out to both the cable and the Bluetooth link, so the
// same messages appear whichever one you happen to be listening on. Print
// gives us println() and printf() for free once write() is supplied.
class MirrorPrint : public Print {
 public:
  size_t write(uint8_t c) override {
    Serial.write(c);
    if (btReady) SerialBT.write(c);
    return 1;
  }
  size_t write(const uint8_t* buf, size_t size) override {
    Serial.write(buf, size);
    if (btReady) SerialBT.write(buf, size);
    return size;
  }
};
MirrorPrint Log;

// What the LED is saying right now.
enum LedState { CONNECTING, ONLINE, OFFLINE };
LedState ledState = CONNECTING;

unsigned long lastCheck = 0;
unsigned long flashUntil = 0;          // brief solid flash when a page is served
unsigned long lastToggle = 0;
bool ledOn = false;

// The web page's button queues a burst of toggles here. One tick is half a
// blink, so 10 ticks at 150 ms is 5 visible blinks over about 1.5 s.
const unsigned long blinkInterval = 150;
int blinkTicksLeft = 0;
unsigned long lastBlinkTick = 0;

// Non-blocking, so the web server keeps answering while the LED blinks.
//   connecting -> fast blink      online -> steady on
//   offline    -> slow blink      request served -> blinks off for a moment
//   button clicked -> 5 quick blinks, which outrank everything above
void updateLed() {
  if (blinkTicksLeft > 0) {
    if (millis() - lastBlinkTick >= blinkInterval) {
      lastBlinkTick = millis();
      ledOn = !ledOn;
      digitalWrite(ledPin, ledOn);
      blinkTicksLeft--;
    }
    return;                     // the burst owns the LED until it runs out
  }

  if (millis() < flashUntil) {
    digitalWrite(ledPin, LOW);
    return;
  }

  if (ledState == ONLINE) {
    digitalWrite(ledPin, HIGH);
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
    Log.println("ssid and password are still the placeholders. Set them at "
                   "the top of this file, then upload again.");
    return false;
  }

  ledState = CONNECTING;
  Log.print("Connecting to ");
  Log.println(ssid);

  WiFi.mode(WIFI_STA);          // join a network, don't become one

  // Must come before begin(). If it fails the board still joins, just on DHCP.
  if (useStaticIP && !WiFi.config(staticIP, gateway, subnet, dnsServer)) {
    Log.println("Static IP rejected, falling back to DHCP.");
  }

  WiFi.begin(ssid, password);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < timeout) {
    updateLed();                // blink while we wait
    delay(10);
  }

  if (WiFi.status() != WL_CONNECTED) {
    ledState = OFFLINE;
    Log.println("Failed. Check the name, the password, and that it is 2.4 GHz.");
    return false;
  }

  ledState = ONLINE;
  Log.print("Connected. Open http://");
  Log.print(WiFi.localIP());
  Log.println(" in your browser.");
  return true;
}

void handleRoot() {
  flashUntil = millis() + 120;  // wink so you can see the request land
  File page = LittleFS.open("/index.html", "r");
  if (!page) {
    server.send(500, "text/plain",
                "index.html is not on the filesystem. Run: pio run -e esp32 -t uploadfs");
    return;
  }
  server.streamFile(page, "text/html");
  page.close();
}

void handleStatus() {
  String json = "{";
  json += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
  json += "\"rssi\":" + String(WiFi.RSSI()) + ",";
  json += "\"uptime\":" + String(millis()) + ",";
  json += "\"heap\":" + String(ESP.getFreeHeap());
  json += "}";
  server.send(200, "application/json", json);
}

void handleBlink() {
  blinkTicksLeft = 10;           // the button on the web page lands here
  lastBlinkTick = 0;             // toggle on the very next updateLed()
  ledOn = true;                  // when ONLINE the LED is already lit, so start
                                 // from "on" and let the first toggle darken it
  Log.println("/blink request received");
  server.send(200, "text/plain", "ok");
}

// Opens the wireless upload listener, so this sketch can be replaced over WiFi
// instead of over the cable. See src/ESP32/001-OTA/README.md for the details;
// the short version is that OTA is not a mode the board sits in, it is code
// that has to be present in whatever firmware is running. Flash something
// without this and the board goes deaf until you plug the cable back in.
void startOTA() {
  ArduinoOTA.setHostname("esp32");
  ArduinoOTA.setPassword(OTA_PASSWORD);   // must match --auth= in platformio.ini

  ArduinoOTA.onStart([]() {
    // Stop serving pages mid-upload, and stop the LED fighting the progress
    // display below.
    server.stop();
    blinkTicksLeft = 0;
    flashUntil = 0;
    Log.println(ArduinoOTA.getCommand() == U_FLASH
                       ? "OTA starting: firmware"
                       : "OTA starting: filesystem");
  });

  ArduinoOTA.onProgress([](unsigned int done, unsigned int total) {
    unsigned int step = total / 20 + 1;
    digitalWrite(ledPin, (done / step) % 2);
    Log.printf("  %u%%\r", total ? (done * 100U) / total : 0);
  });

  ArduinoOTA.onEnd([]() {
    Log.println("\nOTA done, rebooting into the new firmware.");
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Log.printf("\nOTA failed (%u): ", error);
    switch (error) {
      case OTA_AUTH_ERROR:    Log.println("wrong password"); break;
      case OTA_BEGIN_ERROR:   Log.println("no room in the spare OTA slot"); break;
      case OTA_CONNECT_ERROR: Log.println("could not reach the board"); break;
      case OTA_RECEIVE_ERROR: Log.println("transfer cut off partway"); break;
      case OTA_END_ERROR:     Log.println("flash write did not verify"); break;
      default:                Log.println("unknown"); break;
    }
    server.begin();            // a failed upload should not leave the site down
  });

  ArduinoOTA.begin();
  Log.println("OTA listening - upload with: pio run -e esp32-air -t upload");
}

// ---------------------------------------------------------------------------
// Uploading firmware over Bluetooth.
//
// The ROM bootloader cannot do this - it only ever speaks UART, and it runs
// instead of this sketch, so Bluetooth does not exist while it is in charge.
// The trick is to not involve the bootloader at all: this running sketch takes
// the new binary in over Bluetooth and writes it into the spare OTA slot
// itself, using the same Update library that ArduinoOTA drives over WiFi.
// Only the pipe is different.
//
// The other half is tools/bt_upload.py, which platformio.ini runs as a custom
// upload command for the esp32-bt env. Protocol, in full:
//
//   host -> "BTOTA <size> <password>\n"
//   esp  -> "READY\n"          (or "ERR <reason>\n" and we are done)
//   host -> <size> bytes, in 1024-byte chunks
//   esp  -> "K" after each chunk it has written
//   esp  -> "OK\n", then reboots into the new firmware
//
// The per-chunk ack is flow control, not paranoia. Flash writes are slower
// than the link, and without it the sender overruns the receive queue and the
// image silently ends up corrupt.
// ---------------------------------------------------------------------------
String btLine;

void receiveFirmwareOverBluetooth(const String& header) {
  int sp1 = header.indexOf(' ');
  int sp2 = header.indexOf(' ', sp1 + 1);
  if (sp1 < 0 || sp2 < 0) {
    SerialBT.println("ERR malformed header");
    return;
  }

  size_t size = (size_t)header.substring(sp1 + 1, sp2).toInt();
  String pass = header.substring(sp2 + 1);
  pass.trim();

  // Anything paired with the board could otherwise push its own firmware.
  if (pass != String(OTA_PASSWORD)) {
    SerialBT.println("ERR wrong password");
    return;
  }
  if (size == 0 || !Update.begin(size)) {
    SerialBT.printf("ERR cannot begin (%u bytes, error %u)\n", size, Update.getError());
    return;
  }

  server.stop();                 // stop serving pages for the duration
  blinkTicksLeft = 0;
  flashUntil = 0;
  Log.printf("Bluetooth upload starting, %u bytes\n", size);
  SerialBT.println("READY");

  const size_t CHUNK = 1024;
  uint8_t buf[CHUNK];
  size_t got = 0;
  unsigned long lastByte = millis();

  while (got < size) {
    size_t want = (size - got < CHUNK) ? (size - got) : CHUNK;
    size_t have = 0;
    while (have < want) {
      if (SerialBT.available()) {
        buf[have++] = (uint8_t)SerialBT.read();
        lastByte = millis();
      } else if (millis() - lastByte > 15000) {
        Update.abort();
        SerialBT.println("ERR timed out waiting for data");
        Log.println("Bluetooth upload timed out.");
        server.begin();
        return;
      }
    }

    if (Update.write(buf, want) != want) {
      Update.abort();
      SerialBT.println("ERR flash write failed");
      Log.println("Bluetooth upload failed on write.");
      server.begin();
      return;
    }

    got += want;
    SerialBT.write('K');         // "chunk is on the flash, send the next one"
    digitalWrite(ledPin, (got / (size / 20 + 1)) % 2);
  }

  if (!Update.end(true)) {
    SerialBT.printf("ERR did not verify (%u)\n", Update.getError());
    Log.println("Bluetooth upload did not verify.");
    server.begin();
    return;
  }

  SerialBT.println("OK");
  SerialBT.flush();
  Log.println("Bluetooth upload done, rebooting into the new firmware.");
  delay(300);                    // let the last bytes drain before the reset
  ESP.restart();
}

// Watches the Bluetooth link for the one command we care about. Everything
// else arriving on it is ignored, so the link stays usable as a plain monitor.
void handleBluetoothUpload() {
  if (!btReady) {
    return;
  }
  while (SerialBT.available()) {
    char c = (char)SerialBT.read();
    if (c == '\n') {
      String line = btLine;
      btLine = "";
      line.trim();
      if (line.startsWith("BTOTA ")) {
        receiveFirmwareOverBluetooth(line);
      }
      return;
    }
    if (btLine.length() < 120) {  // ignore anything absurd rather than grow
      btLine += c;
    }
  }
}

void setup() {
  pinMode(ledPin, OUTPUT);
  Serial.begin(115200);
  delay(1000);                   // let the monitor open after reset

  // The name that shows up when you go to pair from Windows. Do this before
  // WiFi so the very first log lines are already being mirrored. Classic
  // Bluetooth and WiFi share one radio, so both work at once but each gets
  // less airtime - if uploads or page loads feel sluggish, this is why.
  btReady = SerialBT.begin("ESP32-Monitor");
  Log.println(btReady ? "Bluetooth serial up as \"ESP32-Monitor\"."
                      : "Bluetooth serial failed to start.");

  if (!LittleFS.begin(true)) {   // true = format if it has never been used
    Log.println("LittleFS failed to mount.");
  }

  connectToWiFi();

  // Before the mDNS block below, on purpose. ArduinoOTA.begin() brings mDNS up
  // itself and advertises _arduino._tcp on it, which is what makes the board
  // appear under "Network ports" in the Arduino IDE. Starting it first means
  // one responder ends up carrying both that and the http advert. (A second
  // MDNS.begin() with the same name is harmless - it returns early rather than
  // tearing down what is already registered.)
  if (WiFi.status() == WL_CONNECTED) {
    startOTA();
  }

  // Lets phones and laptops on the same WiFi reach the board by name, so you
  // do not have to retype the IP every time the router hands out a new one.
  // Android is the one holdout that ignores .local - use the IP there.
  if (WiFi.status() == WL_CONNECTED && MDNS.begin("esp32")) {
    MDNS.addService("http", "tcp", 80);
    Log.println("Also reachable at http://esp32.local");
  }

  server.on("/", handleRoot);
  server.on("/status", handleStatus);
  server.on("/blink", handleBlink);
  server.begin();
  Log.println("Web server running on port 80.");
}

void loop() {
  // Has to run every pass, same as handleClient(). Anything that blocks in
  // here - a long delay(), a slow sensor read - starves it and makes wireless
  // uploads time out partway through.
  ArduinoOTA.handle();
  handleBluetoothUpload();
  server.handleClient();
  updateLed();

  if (millis() - lastCheck < checkEvery) {
    return;
  }
  lastCheck = millis();

  if (WiFi.status() == WL_CONNECTED) {
    ledState = ONLINE;
  } else {
    ledState = OFFLINE;
    Log.println("Dropped, reconnecting...");
    if (connectToWiFi()) {
      startOTA();               // the listener does not survive a reconnect
    }
  }
}
