#include <Servo.h>

Servo throttleServo;

const int BRAKE_CLUTCH_SWITCH_PIN = 2;
const int THROTTLE_SERVO_PIN = 9;
const unsigned long MAX_BLIP_DURATION = 300;
const unsigned long BLIP_CANCEL = 0;


// Position of the servo where the throttle is closed
// (the servo does not touch the throttle)
const int NO_THROTTLE_POSITION = 42;

// Position of the servo where the throttle is in
// heel and toe state (throttle blip)
const int BLIP_THROTTLE_POSITION = 60;

// Last state of the brake and clutch switch
boolean lastState = LOW;
// Current state of the brake and clutch switch
boolean currentState = LOW;
// Holds the start time of the throttle blip
unsigned long blipStartTime;

/**
 * Reads the state of the brake/clutch switches.
 * Software debounce is performed in order to get a stable reading.
 *
 * @return the state of the brake/clutch switch
 */
boolean readBrakeAndClutchState() {
  boolean newState = digitalRead(BRAKE_CLUTCH_SWITCH_PIN);
  if (lastState != newState){
    delay(5);
    newState = digitalRead(BRAKE_CLUTCH_SWITCH_PIN);
  }
  return newState;
}

void setup() {
  throttleServo.attach(THROTTLE_SERVO_PIN);

  // Make sure that initially the servo will not interfere
  // with the throttle
  throttleServo.write(NO_THROTTLE_POSITION);
}

void loop() {
  currentState = readBrakeAndClutchState();
  if (lastState != currentState) {
    // brake and clutch switch state changed, so apply throttle
    // depending on the current state
    if (currentState == HIGH){
      blipStartTime = millis();
      throttleServo.write(BLIP_THROTTLE_POSITION);
    } else {
      throttleServo.write(NO_THROTTLE_POSITION);
    }
  } else if (blipStartTime != BLIP_CANCEL && currentState == HIGH && millis() - blipStartTime > MAX_BLIP_DURATION) {
      throttleServo.write(NO_THROTTLE_POSITION);
      blipStartTime = BLIP_CANCEL;
  }
  // if the switch state does not change and there is no blip cancellation condition
  // there will be no change in the state of the servo
  
  // in any case store the current state
  lastState = currentState;
}
