#define MQ2_PIN A0
#define FLAME_PIN 2
#define BUZZER_PIN 8
#define LED_PIN 9

// Adjust this threshold after testing
#define GAS_THRESHOLD 600

int gasValue = 0;
int flameValue = HIGH;

void setup() {
  Serial.begin(9600);

  pinMode(MQ2_PIN, INPUT);
  pinMode(FLAME_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_PIN, LOW);

  Serial.println("SMART FIRE AND GAS DETECTION SYSTEM");
  Serial.println("System Initializing...");
  delay(2000);
}

void loop() {

  gasValue = analogRead(MQ2_PIN);
  flameValue = digitalRead(FLAME_PIN);

  bool gasDetected = gasValue >= GAS_THRESHOLD;
  bool flameDetected = (flameValue == LOW);

  Serial.print("Gas Value: ");
  Serial.print(gasValue);

  Serial.print(" | Flame: ");
  Serial.print(flameDetected ? "DETECTED" : "NOT DETECTED");

  if (gasDetected || flameDetected) {

    Serial.print(" | WARNING: ");

    if (gasDetected && flameDetected) {
      Serial.println("GAS AND FIRE DETECTED!");
    }
    else if (gasDetected) {
      Serial.println("GAS/SMOKE DETECTED!");
    }
    else {
      Serial.println("FLAME DETECTED!");
    }

    digitalWrite(BUZZER_PIN, HIGH);
    digitalWrite(LED_PIN, HIGH);
    delay(250);

    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
    delay(250);

  }
  else {

    Serial.println(" | SAFE");

    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, LOW);

    delay(300);
  }
}
