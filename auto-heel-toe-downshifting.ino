#include <Servo.h>

Servo throttleServo;

const boolean DEBUG_ENABLED = true;
const unsigned long DEBOUNCE_MS = 5;

const int CLUTCH_SWITCH_PIN = 2;
const int BRAKE_SWITCH_PIN = 4;
const int THROTTLE_SERVO_PIN = 9;
const int BLIP_ACTIVE_PIN = 13;

const unsigned long MAX_BLIP_DURATION = 300;
const unsigned long BLIP_CANCEL = 0;


// Position of the servo where the throttle is closed
// (the servo does not touch the throttle)
const int NO_THROTTLE_POSITION = 42;

// Position of the servo where the throttle is in
// heel and toe state (throttle blip)
const int BLIP_THROTTLE_POSITION = 60;

// Last known state of the clutch switch
boolean lastClutchState = LOW;

// Last known state of the brake switch
boolean lastBrakeState = LOW;

// Holds the start time of the throttle blip
unsigned long blipStartTime;

// Holds the start time of clutch pressed event
unsigned long clutchPressedStartTime;

// Holds the start time of brake pressed event
unsigned long brakePressedStartTime;

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
 * Reads the state of the clutch switch.
 * Software debounce is performed in order to get a stable reading.
 *
 * @return the state of theclutch switch
 */
boolean readClutchState() {
  boolean newState = digitalRead(CLUTCH_SWITCH_PIN);
  if (lastClutchState != newState){
    delay(DEBOUNCE_MS);
    newState = digitalRead(CLUTCH_SWITCH_PIN);
  }
  if (DEBUG_ENABLED){
    if (!lastClutchState && newState) {
      clutchPressedStartTime = millis();
    } else if (lastClutchState && !newState) {
      debugLog("Clutch released, duration(ms):" + String(millis() - clutchPressedStartTime, DEC));  
    }
  }
  return newState;
}

/**
 * Reads the state of the brake switch.
 * Software debounce is performed in order to get a stable reading.
 *
 * @return the state of theclutch switch
 */
boolean readBrakeState() {
  boolean newState = digitalRead(BRAKE_SWITCH_PIN);
  if (lastBrakeState != newState){
    delay(DEBOUNCE_MS);
    newState = digitalRead(BRAKE_SWITCH_PIN);
  }
  if (DEBUG_ENABLED){
    if (!lastBrakeState && newState) {
      brakePressedStartTime = millis();
    } else if (lastBrakeState && !newState) {
      debugLog("Brake released, duration(ms):" + String(millis() - brakePressedStartTime, DEC));  
    }
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
    debugLog("\nStaring blip");
    blipStartTime = millis();
    throttleServo.write(BLIP_THROTTLE_POSITION);
    digitalWrite(BLIP_ACTIVE_PIN, HIGH);
  } else if(blipStartTime != BLIP_CANCEL) {
    debugLog("Finishing blip, duration(ms): " + String(millis() - blipStartTime, DEC));
    throttleServo.write(NO_THROTTLE_POSITION);
    digitalWrite(BLIP_ACTIVE_PIN, LOW);
  }
}

void setup() {
  if (DEBUG_ENABLED) {
    Serial.begin(9600);
  }
  pinMode(BLIP_ACTIVE_PIN, OUTPUT);
  throttleServo.attach(THROTTLE_SERVO_PIN);

  // Make sure that initially the servo will not interfere
  // with the throttle
  throttleServo.write(NO_THROTTLE_POSITION);
  digitalWrite(BLIP_ACTIVE_PIN, LOW);
  
  debugLog("Initialized");
}

void loop() {
  boolean clutchState = readClutchState();
  boolean brakeState = readBrakeState();

  if (clutchState != lastClutchState) {
    // TODO on/of the clutch led
    debugLog(clutchState ? "Clutch Pressed": "Clutch Released");
  }

  if (brakeState != lastBrakeState) {
    // TODO on/of the brake led
    debugLog(brakeState ? "Brake Pressed": "Brake Released");
  }

  boolean lastState = lastClutchState && lastBrakeState;
  boolean currentState = clutchState && brakeState;
  if (lastState != currentState) {
      // state changed so apply throttle
      applyThrottle(currentState);
  } else if (shouldStopBlip(currentState)) {
      debugLog("Canceling blip, max duration reached(ms): " + String(MAX_BLIP_DURATION, DEC));

      // Close the throttle and mark the blip as canceled
      throttleServo.write(NO_THROTTLE_POSITION);
      digitalWrite(BLIP_ACTIVE_PIN, LOW);

      blipStartTime = BLIP_CANCEL;
  }
  // if the switch state does not change and there is no blip cancellation condition
  // there will be no change in the throttle servo
  
  // in any case store the current state
  lastClutchState = clutchState;
  lastBrakeState = brakeState;
}
