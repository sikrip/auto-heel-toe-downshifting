#include <Servo.h>

Servo throttleServo;

const boolean DEBUG_ENABLED = true;

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

// Last known state of the brake and clutch switch
boolean lastState = LOW;

// Holds the start time of the throttle blip
unsigned long blipStartTime;

/**
 * If debug is enabled, writes the provided message to
 * the serial port.
 * 
 * @param message the debug message
 */
void debugLog(String message) {
  if (DEBUG_ENABLED) {
    Serial.println(message);
  }
}

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

/**
 * Check if a blip cancellation condition exists. For this to be true, the provided current state
 * must be HIGH, the blip should not be already canceled and the blip duration must be more than
 * {@link MAX_BLIP_DURATION}.
 *
 * @param currentState the current state of the brake/clutch switch (HIGH -> both pressed)
 * @return true if the blip should be canceled, false otherwise
 */
boolean shouldStopBlip(boolean currentState) {
  return blipStartTime != BLIP_CANCEL && currentState == HIGH && millis() - blipStartTime > MAX_BLIP_DURATION;
}

/**
 * Applies throttle based on the provided state of the brake/clutch switch.
 * If it is HIGH(both brake and clutch pressed), then a blip will be applied
 * in the throttle. Otherwise the throttle will be closed.
 *
 * @param currentState the current state of the brake/clutch switch (HIGH -> both pressed)
 */
void applyThrottle(boolean currentState) {
  if (currentState == HIGH) {
    debugLog("Staring blip");
    blipStartTime = millis();
    throttleServo.write(BLIP_THROTTLE_POSITION);
  } else if(blipStartTime != BLIP_CANCEL) {
    debugLog("Ending blip");
    throttleServo.write(NO_THROTTLE_POSITION);
  }
}

void setup() {
  if (DEBUG_ENABLED) {
    Serial.begin(9600);
  }
  throttleServo.attach(THROTTLE_SERVO_PIN);

  // Make sure that initially the servo will not interfere
  // with the throttle
  throttleServo.write(NO_THROTTLE_POSITION);

  debugLog("Initialized");
}

void loop() {
  boolean currentState = readBrakeAndClutchState();
  if (lastState != currentState) {
      // state changed so apply throttle
      applyThrottle(currentState);
  } else if (shouldStopBlip(currentState)) {
      debugLog("Canceling blip");

      // Close the throttle and mark the blip as canceled
      throttleServo.write(NO_THROTTLE_POSITION);
      blipStartTime = BLIP_CANCEL;
  }
  // if the switch state does not change and there is no blip cancellation condition
  // there will be no change in the throttle servo
  
  // in any case store the current state
  lastState = currentState;
}
