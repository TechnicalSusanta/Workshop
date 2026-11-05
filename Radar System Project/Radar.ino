
#include <Servo.h>

Servo radarServo;

const int trigPin = 10;
const int echoPin = 11;
const int servoPin = 9;

int angle = 0;
int direction = 1;

long duration;
float distance;

void setup() {
  Serial.begin(9600);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  radarServo.attach(servoPin);
  radarServo.write(0);

  delay(500);
}

float getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH, 30000);

  if (duration == 0) {
    return 0;
  }

  return duration * 0.0343 / 2.0;
}

void loop() {

  radarServo.write(angle);
  delay(35);

  distance = getDistance();

  // Send angle,distance to Python
  Serial.print(angle);
  Serial.print(",");
  Serial.println(distance, 1);

  angle += direction;

  if (angle >= 180) {
    angle = 180;
    direction = -1;
  }

  if (angle <= 0) {
    angle = 0;
    direction = 1;
  }
}
