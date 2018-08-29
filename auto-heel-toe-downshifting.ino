#include <Servo.h>

Servo throttleServo;

const int brakeAndClutchSwitchPin = 2;
const int throttleServoPin = 9;

// Position of the servo where the throttle is closed
// (the servo does not touch the throttle)
const int offPos = 42;

// Position of the servo where the throttle is in
// heel and toe state (throttle blip)
const int htPos = 60;

// Last state of the brake and clutch switch
boolean lastState = LOW;
// Current state of the brake and clutch switch
boolean currentState = LOW;

/**
 * Reads the state of the brake/clutch switches.
 * Software debounce is performed in order to get a stable reading.
 *
 * @return the state of the brake/clutch switch
 */
boolean readBrakeAndClutchState() {
  boolean newState = digitalRead(brakeAndClutchSwitchPin);
  if (lastState != newState){
    delay(5);
    newState = digitalRead(brakeAndClutchSwitchPin);
  }
  return newState;
}

void setup() {
  throttleServo.attach(throttleServoPin);

  // Make sure that initially the servo will not interfere
  // with the throttle
  throttleServo.write(offPos);
}

void loop() {
  currentState = readBrakeAndClutchState();
  if (lastState != currentState) {
    // brake and clutch switch state changed, so apply throttle
    // depending on the current state
    if (currentState == HIGH){
      throttleServo.write(htPos);
    } else {
      throttleServo.write(offPos);
    }
  }
  // if the switch state does not change, do nothing
  lastState = currentState;
}
