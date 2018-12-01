#include <Bounce2.h>
#include <Servo.h>

// When true, debug messages will be printed
const boolean DEBUG_ENABLED = false;
// The duration in ms of the software debounce performed when reading the 
// state of the switches.
const unsigned long DEBOUNCE_MS = 5;

// Connects to the switch that reads HIGH when the clutch is pressed.
const int CLUTCH_SWITCH_PIN = 2;
// Connects to the LED that indicates that the clutch pedal is pressed.
const int CLUTCH_PRESSED_PIN = 13;

// Connects to the switch that reads HIGH when the brake is pressed.
const int BRAKE_SWITCH_PIN = 4;
// Connects to the LED that indicates that the brake pedal is pressed.
const int BRAKE_PRESSED_PIN = 12;

// Connects to the servo that acts on the throttle
const int THROTTLE_SERVO_PIN = 9;
// Connects to the LED that indicates an active blip
const int BLIP_ACTIVE_PIN = 7;
// Connects to the potentiometer that decides the blip duration(analog pin)
const int BLIP_DURATION_PIN = 0;
// Connects to the potentiometer that decides the blip amount(analog pin)
const int BLIP_AMOUNT_PIN = 1;

// Blip duration cannot be more than this value (ms)
const unsigned long MAX_BLIP_DURATION = 500;
// Blip duration cannot be less tha this valuw (ms)
const unsigned long MIN_BLIP_DURATION = 50;
// Value of the blipStartTime that indicates that the blip is canceled
const unsigned long BLIP_CANCEL = 0;

// Position of the servo where the throttle is closed
// (the servo does not touch the throttle)
const int NO_THROTTLE_POSITION = 41;

// Position of the servo where the throttle is in blipped
// (min blip value)
const int MIN_BLIP_POSITION = 60;

// Position of the servo where the throttle is in blipped
// (max blip value)
const int MAX_BLIP_POSITION = 100;

// Position of the servo where the throttle is in
// heel and toe state (throttle blip)
int blipThrottlePosition;

// Used to open/close the throttle
Servo throttleServo;

// True when the throttle is blipped, false otherwise
boolean throttleBlip = false;

// The allowed blip duration (set via potentiometer)
int blipDuration = 0;

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
    digitalWrite(CLUTCH_PRESSED_PIN, clutchSwitch.rose());
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
    digitalWrite(BRAKE_PRESSED_PIN, brakeSwitch.rose());
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
    if (blipDuration > 0) {
      blipStartTime = millis();
      throttleServo.write(blipThrottlePosition);
      throttleBlip = true;
      digitalWrite(BLIP_ACTIVE_PIN, HIGH);
      debugLog("\nStaring blip");
    } else if (DEBUG_ENABLED){
      debugLog("Auto blip is turned off");
    }
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
 * and the blip duration is be more than {@link blipDuration}.
 */
void maybeCancelBlip() {
    if (throttleBlip && blipStartTime != BLIP_CANCEL && millis() - blipStartTime > blipDuration) {
        // Close the throttle and mark the blip as canceled
        closeThrottle();
        blipStartTime = BLIP_CANCEL;
        if (DEBUG_ENABLED) {
            debugLog("Canceling blip, max duration reached(ms): " + String(blipDuration, DEC));
        }
    }
}

/**
 * Sets the blip duration based on the value read by a potentiometer.
 */
void readBlipDuration() {
  blipDuration = map(analogRead(BLIP_DURATION_PIN), 0, 1023, MIN_BLIP_DURATION, MAX_BLIP_DURATION);
}

/**
 * Sets the blip amount based on the value read by a potentiometer.
 */
void readBlipAmount() {  
  blipThrottlePosition = map(analogRead(BLIP_AMOUNT_PIN), 0, 1023, MIN_BLIP_POSITION, MAX_BLIP_POSITION);
}

void setup() {
    if (DEBUG_ENABLED) {
        Serial.begin(9600);
    }

    clutchSwitch.attach(CLUTCH_SWITCH_PIN);
    clutchSwitch.interval(DEBOUNCE_MS);
    pinMode(CLUTCH_PRESSED_PIN, OUTPUT);

    brakeSwitch.attach(BRAKE_SWITCH_PIN);
    brakeSwitch.interval(DEBOUNCE_MS);
    pinMode(BRAKE_PRESSED_PIN, OUTPUT);

    pinMode(BLIP_ACTIVE_PIN, OUTPUT);
    throttleServo.attach(THROTTLE_SERVO_PIN);

    // Make sure that initially the servo will not interfere with the throttle
    closeThrottle();

    debugLog("Initialized");
}

void loop() {
    readBlipDuration();
    readBlipAmount();
    boolean brakeStateChanged = brakeSwitch.update();
    boolean clutchStateChanged = clutchSwitch.update();
    
    if (clutchStateChanged) {
        markClutchStateChange();
    }
    if (brakeStateChanged) {
        markBrakeStateChange();
    }
    
    if (clutchStateChanged || brakeStateChanged) {
        // State changed so apply throttle according to the current state of the clutch/brake switches
        applyThrottle(brakeSwitch.read() && clutchSwitch.read());
    } else {
        // No change in the clutch/brake state, check and possibly cancel the blip
        maybeCancelBlip();
    }
    // Otherwise there will be no change in the throttle servo
}
