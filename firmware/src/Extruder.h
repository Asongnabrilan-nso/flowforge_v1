#pragma once
#include <Arduino.h>

// Drives the E0 A4988 stepper via Timer1 (CTC mode, ISR toggles STEP).
// Timer1 is otherwise unused by this firmware (buzzer uses tone(), which on
// the Mega runs off Timer2, and the heater uses software PWM - see Heater.h
// for why analogWrite/Timer2 sharing was avoided there).
//
// Direction is fixed forward for this build (a hand pen only needs to feed
// filament); setDirection() is exposed for future expansion (e.g. a
// retract/reverse menu item) without needing to touch the ISR.
class Extruder {
public:
    void begin();

    // level is clamped to [SPEED_LEVEL_MIN, SPEED_LEVEL_MAX] and mapped
    // linearly to a step frequency; takes effect immediately, even while running.
    void setSpeedLevel(uint8_t level);
    uint8_t speedLevel() const { return _level; }

    void setDirection(bool forward);

    void setEnabled(bool enabled);
    bool isEnabled() const { return _enabled; }

private:
    uint8_t _level = 1;
    bool _forward = true;
    bool _enabled = false;
};

extern Extruder extruder;
