#pragma once
#include <Arduino.h>
#include "Thermistor.h"
#include "PID.h"

enum class HeaterState : uint8_t {
    IDLE,       // target is 0, heater off
    HEATING,    // ramping toward target
    AT_TEMP,    // within tolerance of target
    FAULT       // latched safety fault - heater forced off until reset
};

enum class FaultReason : uint8_t {
    NONE,
    SENSOR,     // thermistor open/short
    OVER_TEMP,  // exceeded TEMP_MAX_C
    RUNAWAY_NO_RISE,  // heating but not rising fast enough (watchdog)
    RUNAWAY_DRIFT     // was at temp but drifted away and stayed away
};

// Owns the thermistor + PID + heater output for the hotend, and enforces
// safety limits. All timing is millis()-based and non-blocking: call
// update() often from loop(), it internally rate-limits itself to
// PID_SAMPLE_MS.
//
// Heater output uses a slow software PWM window instead of analogWrite().
// Reason: pin 10 shares Timer2 with the Arduino core's tone() (used for the
// buzzer on pin 33); driving both from hardware PWM/tone at once fights
// over the same timer. A hotend's thermal mass is seconds-long anyway, so a
// millis()-driven on/off window is both simpler and plenty precise.
class Heater {
public:
    Heater(uint8_t heaterPin, Thermistor& thermistor);

    void begin(); // call from setup() - constructor must stay hardware-free
                  // since Heater is instantiated as a global object
    void update();

    void setTarget(float celsius);
    float target() const { return _target; }
    float current() const { return _current; }

    HeaterState state() const { return _state; }
    FaultReason faultReason() const { return _faultReason; }

    // Cold-extrusion interlock: true once current temp is above the
    // configured minimum safe extrusion temperature.
    bool safeToExtrude() const;

private:
    void enterFault(FaultReason reason);
    void applyOutput();

    uint8_t _pin;
    Thermistor& _thermistor;
    PID _pid;

    float _target = 0.0f;
    float _current = 0.0f;
    uint8_t _pwmOutput = 0;

    HeaterState _state = HeaterState::IDLE;
    FaultReason _faultReason = FaultReason::NONE;

    unsigned long _lastSampleMs = 0;
    unsigned long _windowStartMs = 0;

    // Runaway watchdog bookkeeping.
    unsigned long _watchWindowStartMs = 0;
    float _watchWindowStartTemp = 0.0f;
    unsigned long _driftStartMs = 0;
    bool _drifting = false;
};
