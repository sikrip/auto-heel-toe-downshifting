#include <Servo.h>

Servo throttleServo;

const int THROTTLE_CLOSED = 63 cx;
const int THROTTLE_HT = 80;

const int THROTTLE_SERVO_PIN = 9;
const int BRAKE_BUTTON_PIN = 2;

int brakeButtonState = 0;

void setup() {
  throttleServo.attach(THROTTLE_SERVO_PIN);  // attaches the servo on pin 9 to the servo object
  pinMode(BRAKE_BUTTON_PIN, INPUT);
}

void loop() {
  brakeButtonState = digitalRead(BRAKE_BUTTON_PIN);
  if (brakeButtonState == HIGH) {
      throttleServo.write(THROTTLE_HT);
  } else {
      throttleServo.write(THROTTLE_CLOSED);
  }
  delay(10);
}
