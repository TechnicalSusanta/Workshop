
#define MQ3_PIN A0
#define BUZZER 8
#define LED 7

int alcoholThreshold = 800;

void setup() {
  Serial.begin(9600);

  pinMode(MQ3_PIN, INPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(LED, OUTPUT);

  digitalWrite(BUZZER, LOW);
  digitalWrite(LED, LOW);

  Serial.println("Alcohol Detection System");
}

void loop() {

  int alcoholValue = analogRead(MQ3_PIN);

  Serial.print("Alcohol Sensor Value: ");
  Serial.println(alcoholValue);

  if (alcoholValue > alcoholThreshold) {

    Serial.println("ALCOHOL DETECTED!");

    digitalWrite(BUZZER, HIGH);
    digitalWrite(LED, HIGH);
    delay(300);

    digitalWrite(BUZZER, LOW);
    digitalWrite(LED, LOW);
    delay(300);

  } 
  else {

    Serial.println("No Alcohol Detected");

    digitalWrite(BUZZER, LOW);
    digitalWrite(LED, LOW);

    delay(500);
  }
}

