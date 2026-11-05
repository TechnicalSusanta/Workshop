
#include <DHT.h>

// -------- PIN CONFIGURATION --------
#define SOIL_PIN A0
#define DHT_PIN 2
#define DHT_TYPE DHT11
#define RELAY_PIN 7

DHT dht(DHT_PIN, DHT_TYPE);

// -------- SOIL CALIBRATION --------
const int SOIL_DRY = 800;
const int SOIL_WET = 300;

// -------- SETTINGS --------
int moistureThreshold = 35;
const int HYSTERESIS = 5;

// Most relay modules are active LOW
#define RELAY_ACTIVE_LOW true

bool pumpStatus = false;
bool autoMode = true;

unsigned long previousMillis = 0;
const unsigned long interval = 2000;

// -------- PUMP CONTROL --------
void controlPump(bool state) {
  pumpStatus = state;

  if (RELAY_ACTIVE_LOW) {
    digitalWrite(RELAY_PIN, state ? LOW : HIGH);
  } else {
    digitalWrite(RELAY_PIN, state ? HIGH : LOW);
  }
}

// -------- SOIL MOISTURE --------
int readSoilMoisture() {
  int rawValue = analogRead(SOIL_PIN);

  int moisture = map(
    rawValue,
    SOIL_DRY,
    SOIL_WET,
    0,
    100
  );

  return constrain(moisture, 0, 100);
}

// -------- SERIAL COMMANDS --------
void processCommand(String command) {
  command.trim();
  command.toUpperCase();

  if (command == "AUTO") {
    autoMode = true;
  }

  else if (command == "ON") {
    autoMode = false;
    controlPump(true);
  }

  else if (command == "OFF") {
    autoMode = false;
    controlPump(false);
  }

  else if (command.startsWith("THRESHOLD:")) {
    int value = command.substring(10).toInt();

    if (value >= 5 && value <= 90) {
      moistureThreshold = value;
    }
  }
}

// -------- SETUP --------
void setup() {
  Serial.begin(9600);

  digitalWrite(RELAY_PIN, RELAY_ACTIVE_LOW ? HIGH : LOW);
  pinMode(RELAY_PIN, OUTPUT);

  controlPump(false);

  dht.begin();

  delay(1000);

  Serial.println("SMART IRRIGATION SYSTEM STARTED");
}

// -------- MAIN LOOP --------
void loop() {

  // Receive commands from web dashboard
  while (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    processCommand(command);
  }

  // Read sensors
  int soil = readSoilMoisture();

  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  // Automatic pump control
  if (autoMode) {

    if (!pumpStatus && soil < moistureThreshold) {
      controlPump(true);
    }

    else if (pumpStatus &&
             soil >= moistureThreshold + HYSTERESIS) {
      controlPump(false);
    }
  }

  // Send data to web dashboard every 2 seconds
  if (millis() - previousMillis >= interval) {

    previousMillis = millis();

    Serial.print("{\"soil\":");
    Serial.print(soil);

    Serial.print(",\"raw\":");
    Serial.print(analogRead(SOIL_PIN));

    Serial.print(",\"humidity\":");
    if (isnan(humidity)) Serial.print("null");
    else Serial.print(humidity, 1);

    Serial.print(",\"temperature\":");
    if (isnan(temperature)) Serial.print("null");
    else Serial.print(temperature, 1);

    Serial.print(",\"pump\":\"");
    Serial.print(pumpStatus ? "ON" : "OFF");

    Serial.print("\",\"mode\":\"");
    Serial.print(autoMode ? "AUTO" : "MANUAL");

    Serial.print("\",\"threshold\":");
    Serial.print(moistureThreshold);

    Serial.println("}");
  }
}

