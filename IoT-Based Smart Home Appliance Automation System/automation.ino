
/*
  ESP32 4-Relay Web Server
  - No Sinric Pro
  - No cloud connection required
  - Control from a web browser using the ESP32 IP
  - Four relays + four physical switches
  - Automatically starts an ESP32 access point if router Wi-Fi fails
*/

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

// -------- YOUR HOME / OFFICE WIFI --------
#define WIFI_SSID     "ERAPOWER"
#define WIFI_PASS     "Susanta1"

// -------- FALLBACK ESP32 HOTSPOT --------
const char* AP_SSID = "ESP32-Relay-Control";
const char* AP_PASS = "relay1234";

// -------- RELAY PINS --------
const uint8_t RELAY_COUNT = 4;

const uint8_t relayPins[RELAY_COUNT] = {
  23, 22, 21, 19
};

// -------- PHYSICAL SWITCH PINS --------
const uint8_t switchPins[RELAY_COUNT] = {
  13, 12, 14, 27
};

const uint8_t wifiLed = 2;

// Most relay modules are active LOW.
// Set to 0 if your relay module is active HIGH.
#define RELAY_ACTIVE_LOW 1

// Uncomment this line when using momentary push buttons.
// Leave commented for maintained toggle switches.
// #define TACTILE_BUTTON

const unsigned long DEBOUNCE_TIME = 50;

WebServer server(80);

// Logical relay states: false = OFF, true = ON
bool relayStates[RELAY_COUNT] = {
  false, false, false, false
};

// Physical switch debounce variables
bool rawSwitchState[RELAY_COUNT];
bool stableSwitchState[RELAY_COUNT];

unsigned long lastSwitchChange[RELAY_COUNT];

// -------- WEB DASHBOARD --------
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<meta name="theme-color" content="#101827">

<title>ESP32 Relay Control</title>

<style>
  * { box-sizing: border-box; }

  body {
    margin: 0;
    padding: 24px 16px;
    background: #101827;
    color: #f4f7fb;
    font-family: Arial, Helvetica, sans-serif;
  }

  .container {
    max-width: 850px;
    margin: auto;
  }

  header {
    margin: 12px 0 26px;
  }

  h1 {
    margin-bottom: 8px;
    font-size: clamp(25px, 5vw, 36px);
  }

  .subtitle {
    color: #a8b5c9;
    line-height: 1.6;
  }

  .connection {
    display: inline-block;
    margin-top: 10px;
    border-radius: 20px;
    padding: 7px 12px;
    background: #293346;
    color: #d3dceb;
    font-size: 13px;
  }

  .connection.live {
    background: #153d32;
    color: #8ef0c2;
  }

  .grid {
    display: grid;
    grid-template-columns: repeat(2, minmax(0, 1fr));
    gap: 16px;
  }

  .card {
    padding: 22px;
    border: 1px solid #344056;
    border-radius: 18px;
    background: #192438;
    transition: border-color .2s;
  }

  .card.on {
    border-color: #20c997;
  }

  .relay-top {
    display: flex;
    align-items: center;
    justify-content: space-between;
    gap: 10px;
  }

  .relay-name {
    font-weight: 700;
    font-size: 19px;
  }

  .relay-number {
    margin-top: 7px;
    font-size: 13px;
    color: #9cabc1;
  }

  .status {
    display: inline-block;
    min-width: 55px;
    text-align: center;
    padding: 6px 10px;
    border-radius: 20px;
    font-size: 12px;
    font-weight: 700;
    background: #374151;
    color: #e0e7f0;
  }

  .card.on .status {
    color: #83f1c2;
    background: #14533f;
  }

  button {
    width: 100%;
    margin-top: 24px;
    padding: 13px;
    border: 0;
    border-radius: 11px;
    background: #344056;
    color: #fff;
    font-weight: 700;
    font-size: 15px;
    cursor: pointer;
  }

  .card.on button {
    background: #087f5b;
  }

  button:disabled {
    opacity: .55;
    cursor: wait;
  }

  footer {
    margin-top: 25px;
    color: #93a2b8;
    font-size: 12px;
    line-height: 1.7;
    text-align: center;
  }

  @media(max-width: 480px) {
    .grid { grid-template-columns: 1fr; }
    .card { padding: 18px; }
  }
</style>
</head>

<body>
<div class="container">

  <header>
    <h1>ESP32 Relay Control</h1>
    <div class="subtitle">
      Local smart home dashboard<br>
      Control your four relays from this device.
    </div>
    <div id="connection" class="connection">
      Connecting to ESP32...
    </div>
  </header>

  <main class="grid">

    <section class="card" id="card1">
      <div class="relay-top">
        <div>
          <div class="relay-name">Relay 1</div>
          <div class="relay-number">GPIO 23</div>
        </div>
        <span class="status" id="status1">OFF</span>
      </div>
      <button id="btn1" onclick="toggleRelay(1)">Turn ON</button>
    </section>

    <section class="card" id="card2">
      <div class="relay-top">
        <div>
          <div class="relay-name">Relay 2</div>
          <div class="relay-number">GPIO 22</div>
        </div>
        <span class="status" id="status2">OFF</span>
      </div>
      <button id="btn2" onclick="toggleRelay(2)">Turn ON</button>
    </section>

    <section class="card" id="card3">
      <div class="relay-top">
        <div>
          <div class="relay-name">Relay 3</div>
          <div class="relay-number">GPIO 21</div>
        </div>
        <span class="status" id="status3">OFF</span>
      </div>
      <button id="btn3" onclick="toggleRelay(3)">Turn ON</button>
    </section>

    <section class="card" id="card4">
      <div class="relay-top">
        <div>
          <div class="relay-name">Relay 4</div>
          <div class="relay-number">GPIO 19</div>
        </div>
        <span class="status" id="status4">OFF</span>
      </div>
      <button id="btn4" onclick="toggleRelay(4)">Turn ON</button>
    </section>

  </main>

  <footer>
    ESP32 Local Web Server · No cloud service required<br>
    Dashboard state refreshes automatically.
  </footer>

</div>

<script>
  let relayStates = [false, false, false, false];

  function renderDashboard() {
    for (let i = 0; i < 4; i++) {
      const id = i + 1;
      const isOn = relayStates[i];

      document.getElementById('card' + id)
        .classList.toggle('on', isOn);

      document.getElementById('status' + id).textContent =
        isOn ? 'ON' : 'OFF';

      document.getElementById('btn' + id).textContent =
        isOn ? 'Turn OFF' : 'Turn ON';
    }
  }

  async function refreshState() {
    try {
      const response = await fetch('/api/state', {
        cache: 'no-store'
      });

      if (!response.ok) {
        throw new Error('Connection failed');
      }

      const data = await response.json();
      relayStates = data.relays;

      renderDashboard();

      const connection = document.getElementById('connection');
      connection.textContent = 'Connected to ESP32';
      connection.className = 'connection live';

    } catch (error) {
      const connection = document.getElementById('connection');
      connection.textContent = 'Connection lost - retrying...';
      connection.className = 'connection';
    }
  }

  async function toggleRelay(id) {
    const button = document.getElementById('btn' + id);
    button.disabled = true;

    try {
      const response = await fetch('/api/toggle?id=' + id, {
        cache: 'no-store'
      });

      if (!response.ok) {
        throw new Error('Relay command failed');
      }

      await refreshState();

    } catch (error) {
      document.getElementById('connection').textContent =
        'Command failed - check the connection';
    } finally {
      button.disabled = false;
    }
  }

  refreshState();
  setInterval(refreshState, 1000);
</script>
</body>
</html>
)rawliteral";

// -------- SET RELAY STATE --------
void setRelay(uint8_t index, bool state) {
  if (index >= RELAY_COUNT) {
    return;
  }

  relayStates[index] = state;

#if RELAY_ACTIVE_LOW
  digitalWrite(relayPins[index], state ? LOW : HIGH);
#else
  digitalWrite(relayPins[index], state ? HIGH : LOW);
#endif

  Serial.printf(
    "Relay %u: %s\n",
    index + 1,
    state ? "ON" : "OFF"
  );
}

void toggleRelay(uint8_t index) {
  if (index >= RELAY_COUNT) {
    return;
  }

  setRelay(index, !relayStates[index]);
}

// -------- INITIALIZE RELAYS --------
void setupRelays() {
  for (uint8_t i = 0; i < RELAY_COUNT; i++) {
    pinMode(relayPins[i], OUTPUT);
    setRelay(i, false);
  }
}

// -------- INITIALIZE PHYSICAL SWITCHES --------
void setupSwitches() {
  for (uint8_t i = 0; i < RELAY_COUNT; i++) {
    pinMode(switchPins[i], INPUT_PULLUP);

    bool currentState = digitalRead(switchPins[i]);

    rawSwitchState[i] = currentState;
    stableSwitchState[i] = currentState;
    lastSwitchChange[i] = millis();
  }
}

// -------- READ PHYSICAL SWITCHES --------
void handlePhysicalSwitches() {
  unsigned long now = millis();

  for (uint8_t i = 0; i < RELAY_COUNT; i++) {
    bool reading = digitalRead(switchPins[i]);

    if (reading != rawSwitchState[i]) {
      rawSwitchState[i] = reading;
      lastSwitchChange[i] = now;
    }

    if ((now - lastSwitchChange[i]) >= DEBOUNCE_TIME &&
        reading != stableSwitchState[i]) {

      stableSwitchState[i] = reading;

#ifdef TACTILE_BUTTON
      // INPUT_PULLUP: LOW means the button is pressed.
      if (stableSwitchState[i] == LOW) {
        toggleRelay(i);
      }
#else
      // Maintained toggle switch: toggle the relay
      // whenever the switch changes position.
      toggleRelay(i);
#endif
    }
  }
}

// -------- WEBPAGE ROUTES --------
void setupWebServer() {

  // Main dashboard
  server.on("/", HTTP_GET, []() {
    server.send_P(200, "text/html", INDEX_HTML);
  });

  // Return current states as JSON
  server.on("/api/state", HTTP_GET, []() {
    String json = "{\"relays\":[";

    for (uint8_t i = 0; i < RELAY_COUNT; i++) {
      if (i > 0) {
        json += ",";
      }

      json += relayStates[i] ? "true" : "false";
    }

    json += "]}";

    server.sendHeader("Cache-Control", "no-store");
    server.send(200, "application/json", json);
  });

  // Toggle an individual relay
  server.on("/api/toggle", HTTP_GET, []() {

    if (!server.hasArg("id")) {
      server.send(400, "application/json",
                  "{\"error\":\"Missing relay id\"}");
      return;
    }

    int id = server.arg("id").toInt();

    if (id < 1 || id > RELAY_COUNT) {
      server.send(400, "application/json",
                  "{\"error\":\"Invalid relay id\"}");
      return;
    }

    toggleRelay(id - 1);

    server.send(200, "application/json", "{\"ok\":true}");
  });

  server.onNotFound([]() {
    server.send(404, "text/plain", "404 - Page not found");
  });

  server.begin();

  Serial.println("Web server started on port 80");
}

// -------- WIFI / FALLBACK ACCESS POINT --------
void setupWiFi() {
  Serial.println();
  Serial.println("Connecting to your Wi-Fi router...");

  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  unsigned long startTime = millis();

  // Wait up to 15 seconds for the router
  while (WiFi.status() != WL_CONNECTED &&
         millis() - startTime < 15000) {

    delay(300);
    Serial.print(".");
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Wi-Fi connected!");
    Serial.print("ESP32 IP address: ");
    Serial.println(WiFi.localIP());

    Serial.print("Open this URL: http://");
    Serial.println(WiFi.localIP());

  } else {
    Serial.println("Router connection failed.");
    Serial.println("Starting ESP32 fallback hotspot...");

    WiFi.mode(WIFI_AP);

    if (WiFi.softAP(AP_SSID, AP_PASS)) {
      Serial.println("ESP32 hotspot started.");
      Serial.print("Wi-Fi name: ");
      Serial.println(AP_SSID);

      Serial.print("Wi-Fi password: ");
      Serial.println(AP_PASS);

      Serial.print("Open this URL: http://");
      Serial.println(WiFi.softAPIP());
    } else {
      Serial.println("Failed to start the hotspot!");
    }
  }

  digitalWrite(wifiLed, HIGH);
}

// -------- SETUP --------
void setup() {
  Serial.begin(115200);

  pinMode(wifiLed, OUTPUT);
  digitalWrite(wifiLed, LOW);

  setupRelays();
  setupSwitches();

  setupWiFi();
  setupWebServer();

  Serial.println("System ready.");
}

// -------- MAIN LOOP --------
void loop() {
  server.handleClient();
  handlePhysicalSwitches();

  delay(1);
}
