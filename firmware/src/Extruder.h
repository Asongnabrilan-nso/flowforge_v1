#pragma once
#include <Arduino.h>

// Drives the E0 A4988 stepper via Timer1 (CTC mode, ISR toggles STEP).
// Timer1 is otherwise unused by this firmware (buzzer uses tone(), which on
// the Mega runs off Timer2, and the heater uses software PWM - see Heater.h
// for why analogWrite/Timer2 sharing was avoided there).
//
// Speed is expressed as real physical units (steps/mm, mm/s, mm/s^2 -
// see config.h's "Extruder motion" block), Marlin-style, instead of a raw
// step frequency. setSpeedLevel()/setEnabled() only set a *target*; update()
// (call every loop()) is what actually ramps the motor there at an
// acceleration-limited rate and writes the timer/enable-pin hardware - this
// gives a soft start/stop instead of an instant frequency jump, reducing
// stall risk and matching Marlin's trapezoidal-ramp motion model.
//
// Direction is fixed forward for this build (a hand pen only needs to feed
// filament); setDirection() is exposed for future expansion (e.g. a
// retract/reverse menu item) without needing to touch the ISR.
class Extruder {
public:
    void begin();

    // level is clamped to [SPEED_LEVEL_MIN, SPEED_LEVEL_MAX]; update() ramps
    // the motor toward the corresponding feedrate, it doesn't jump there.
    void setSpeedLevel(uint8_t level);
    uint8_t speedLevel() const { return _level; }
    static float levelToFeedrateMmS(uint8_t level);

    void setDirection(bool forward);

    // Target on/off state; update() ramps the motor up/down to match rather
    // than switching instantly (see .cpp for why: ramping down before
    // de-energizing the driver preserves holding torque through the stop).
    void setEnabled(bool enabled);
    bool isEnabled() const { return _enabled; }

    void update();                        // call every loop() - advances the accel ramp
    float currentFeedrateMmS() const;     // actual ramped speed, for the status display

private:
    void applyFreq(float hz); // sole writer of OCR1A/TIMSK1/E0_ENABLE_PIN

    uint8_t _level = 1;
    bool _forward = true;
    bool _enabled = false;

    float _currentFreqHz = 0.0f;
    unsigned long _lastUpdateMs = 0;
    bool _driverEnabledPinLow = false; // actual E0_ENABLE_PIN state, decoupled
                                        // from _enabled during ramp-down-then-disable
};

extern Extruder extruder;
