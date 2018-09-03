#include <Bounce2.h>
#include <Servo.h>

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

// Used to open/close the throttle
Servo throttleServo;

// True when the throttle is blipped, false otherwise
boolean throttleBlip = false;

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
 * Moves the throttle servo in the "blip" position.
 */
void blipThrottle() {
    blipStartTime = millis();
    throttleServo.write(BLIP_THROTTLE_POSITION);
    throttleBlip = true;
    digitalWrite(BLIP_ACTIVE_PIN, HIGH);
}

/**
 * Moves the throttle servo in the closed position.
 */
void closeThrottle() {
    throttleServo.write(NO_THROTTLE_POSITION);
    throttleBlip = false;
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
        blipThrottle();
        debugLog("\nStaring blip");
    } else if (blipStartTime != BLIP_CANCEL) {
        closeThrottle();
        if (DEBUG_ENABLED) {
            debugLog("Ending blip, duration(ms): " + String(millis() - blipStartTime, DEC));
        }
    }
}

/**
 * Checks if a blip cancellation condition exists and if this is true, closes the throttle.
 * Such a condition is true when the throttle is blipped, the blip is not be already canceled
 * and the blip duration is be more than {@link MAX_BLIP_DURATION}.
 */
void maybeCancelBlip() {
    if (throttleBlip && blipStartTime != BLIP_CANCEL && millis() - blipStartTime > MAX_BLIP_DURATION) {
        // Close the throttle and mark the blip as canceled
        closeThrottle();
        blipStartTime = BLIP_CANCEL;
        if (DEBUG_ENABLED) {
            debugLog("Canceling blip, max duration reached(ms): " + String(MAX_BLIP_DURATION, DEC));
        }
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
    boolean brakeStateChanged = brakeSwitch.update();
    boolean clutchStateChanged = clutchSwitch.update();

    if (clutchStateChanged || brakeStateChanged) {
        // State changed so apply throttle according to the current state of the clutch/brake switches
        applyThrottle(brakeSwitch.read() && clutchSwitch.read());
    } else {
        // No change in the clutch/brake state, check and possibly cancel the blip
        maybeCancelBlip();
    } // Otherwise there will be no change in the throttle servo

    // Non-crucial operations
    if (clutchStateChanged) {
        markClutchStateChange();
    }
    if (brakeStateChanged) {
        markBrakeStateChange();
    }
}
