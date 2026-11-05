
#include <Servo.h>

Servo rainServo;

const int rainSensor = A0;
const int servoPin = 9;

int rainValue = 0;
int rainThreshold = 900;

void setup() {
  Serial.begin(9600);

  rainServo.attach(servoPin);
  rainServo.write(0);

  Serial.println("Rain Detection System Started");
}

void loop() {

  rainValue = analogRead(rainSensor);

  Serial.print("Rain Sensor Value: ");
  Serial.println(rainValue);

  if (rainValue < rainThreshold) {

    // Rain detected
    rainServo.write(90);

    Serial.println("RAIN DETECTED!");
    Serial.println("Servo Rotated to 90 Degree");

  } 
  else {

    // No rain detected
    rainServo.write(0);

    Serial.println("NO RAIN");
    Serial.println("Servo Returned to 0 Degree");
  }

  delay(500);
}

