#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <LittleFS.h>

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
    Serial.println("ssid and password are still the placeholders. Set them at "
                   "the top of this file, then upload again.");
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
    return false;
  }

  ledState = ONLINE;
  Serial.print("Connected. Open http://");
  Serial.print(WiFi.localIP());
  Serial.println(" in your browser.");
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
  Serial.println("/blink request received");
  server.send(200, "text/plain", "ok");
}

void setup() {
  pinMode(ledPin, OUTPUT);
  Serial.begin(115200);
  delay(1000);                   // let the monitor open after reset

  if (!LittleFS.begin(true)) {   // true = format if it has never been used
    Serial.println("LittleFS failed to mount.");
  }

  connectToWiFi();

  // Lets phones and laptops on the same WiFi reach the board by name, so you
  // do not have to retype the IP every time the router hands out a new one.
  // Android is the one holdout that ignores .local - use the IP there.
  if (WiFi.status() == WL_CONNECTED && MDNS.begin("esp32")) {
    MDNS.addService("http", "tcp", 80);
    Serial.println("Also reachable at http://esp32.local");
  }

  server.on("/", handleRoot);
  server.on("/status", handleStatus);
  server.on("/blink", handleBlink);
  server.begin();
  Serial.println("Web server running on port 80.");
}

void loop() {
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
    Serial.println("Dropped, reconnecting...");
    connectToWiFi();
  }
}
