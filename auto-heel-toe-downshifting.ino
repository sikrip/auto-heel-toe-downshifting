#include <Bounce2.h>
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

// The clutch switch
Bounce clutchSwitch = Bounce();
// Holds the start time of clutch pressed event
unsigned long clutchPressedStartTime;

// The brake switch
Bounce brakeSwitch = Bounce();
// Holds the start time of brake pressed event
unsigned long brakePressedStartTime;

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
 * Marks the clutch as pressed / released.
 * Not crucial of the blip operation, just open close the LED and debug info.
 */
void markClutchStateChange() {
  // TODO open/close the LED
  if (DEBUG_ENABLED) {
    if (clutchSwitch.rose()) {
      clutchPressedStartTime = millis();
      debugLog("Clutch pressed");
    } else if (clutchSwitch.fell()) {
      debugLog("Clutch released, duration(ms):" + String(millis() - clutchPressedStartTime, DEC));
    }
  }
}

/**
 * Marks the brake as pressed / released.
 * Not crucial of the blip operation, just open close the LED and debug info.
 */
void markBrakeStateChange() {
  // TODO open/close the LED
  if (DEBUG_ENABLED) {
    if (brakeSwitch.rose()) {
      brakePressedStartTime = millis();
      debugLog("Brake pressed");
    } else if (brakeSwitch.fell()) {
      debugLog("Brake released, duration(ms):" + String(millis() - brakePressedStartTime, DEC));
    }
  }
}

/**
 * Check if a blip cancellation condition exists. For this to be true, the provided state
 * must be HIGH, the blip should not be already canceled and the blip duration must be more than
 * {@link MAX_BLIP_DURATION}.
 *
 * @param state the current state of the brake/clutch switch (HIGH -> both pressed)
 * @return true if the blip should be canceled, false otherwise
 */
boolean shouldCancelBlip(boolean state) {
  return blipStartTime != BLIP_CANCEL && state == HIGH && millis() - blipStartTime > MAX_BLIP_DURATION;
}

/**
 * Moves the throttle servo in the "blip" position.
 */
void blipThrottle() {
  blipStartTime = millis();
  throttleServo.write(BLIP_THROTTLE_POSITION);
  digitalWrite(BLIP_ACTIVE_PIN, HIGH);
}

/**
 * Moves the throttle servo in the closed position.
 */
void closeThrottle() {
  throttleServo.write(NO_THROTTLE_POSITION);
  digitalWrite(BLIP_ACTIVE_PIN, LOW);
}

/**
 * Applies throttle based on the provided state of the brake/clutch switch.
 * If it is HIGH(both brake and clutch pressed), then a blip will be applied
 * in the throttle. Otherwise the throttle will be closed.
 *
 * @param state the current state of the brake/clutch switch (HIGH -> both pressed)
 */
void applyThrottle(boolean state) {
  if (state == HIGH) {
    debugLog("\nStaring blip");
    blipThrottle();
  } else if(blipStartTime != BLIP_CANCEL) {
    debugLog("Finishing blip, duration(ms): " + String(millis() - blipStartTime, DEC));
    closeThrottle();
  }
}

void setup() {
  if (DEBUG_ENABLED) {
    Serial.begin(9600);
  }
  
  clutchSwitch.attach(CLUTCH_SWITCH_PIN);
  clutchSwitch.interval(DEBOUNCE_MS);

  brakeSwitch.attach(BRAKE_SWITCH_PIN);
  brakeSwitch.interval(DEBOUNCE_MS);
  
  pinMode(BLIP_ACTIVE_PIN, OUTPUT);
  throttleServo.attach(THROTTLE_SERVO_PIN);

  // Make sure that initially the servo will not interfere with the throttle
  closeThrottle();
  
  debugLog("Initialized");
}

void loop() {
  static boolean brakeStateChanged = brakeSwitch.update();
  static boolean brakeState = brakeSwitch.read();
  static boolean clutchStateChanged = clutchSwitch.update();
  static boolean clutchState = clutchSwitch.read();

  static boolean currentState = clutchState && brakeState;

  if (clutchStateChanged || brakeStateChanged) {
      // state changed so apply throttle
      applyThrottle(currentState);
  } else if (shouldCancelBlip(currentState)) {
      debugLog("Canceling blip, max duration reached(ms): " + String(MAX_BLIP_DURATION, DEC));

      // Close the throttle and mark the blip as canceled
      closeThrottle();
      blipStartTime = BLIP_CANCEL;
  }
  // if the switch state does not change and there is no blip cancellation condition
  // there will be no change in the throttle servo

  if (clutchStateChanged) {
    markClutchStateChange();
  }

  if (brakeStateChanged) {
    markBrakeStateChange();
  }
}
