#include "Heater.h"
#include <math.h>
#include "../include/config.h"

Heater::Heater(uint8_t heaterPin, Thermistor& thermistor)
    : _pin(heaterPin), _thermistor(thermistor),
      // PID_OUTPUT_MAX_CAPPED, not PID_OUTPUT_MAX: derates the ceiling the
      // bang-bang bootstrap and anti-windup clamp both use, so the heater is
      // never driven at raw 100% duty for extended stretches (see config.h).
      _pid(PID_KP, PID_KI, PID_KD, PID_OUTPUT_MIN, PID_OUTPUT_MAX_CAPPED,
           PID_FUNCTIONAL_RANGE_C, PID_K1) {
}

void Heater::begin() {
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);
}

void Heater::setTarget(float celsius) {
    if (_state == HeaterState::FAULT) return; // latched - ignore until reset
    if (celsius < TEMP_SETPOINT_MIN_C) celsius = TEMP_SETPOINT_MIN_C;
    if (celsius > TEMP_MAX_C) celsius = TEMP_MAX_C;
    _target = celsius;
}

bool Heater::safeToExtrude() const {
    return _state != HeaterState::FAULT && _current >= TEMP_MIN_EXTRUDE_C;
}

void Heater::enterFault(FaultReason reason) {
    _state = HeaterState::FAULT;
    _faultReason = reason;
    _pwmOutput = 0;
    digitalWrite(_pin, LOW);
}

void Heater::update() {
    unsigned long now = millis();

    if (_state == HeaterState::FAULT) {
        digitalWrite(_pin, LOW); // belt and braces - never let this drift high
        return;
    }

    // --- Slow control loop: read sensor, run PID, decide state (every PID_SAMPLE_MS) ---
    if (now - _lastSampleMs >= PID_SAMPLE_MS) {
        _lastSampleMs = now;

        _current = _thermistor.readCelsius();
        if (_thermistor.lastReadFaulted()) {
            enterFault(FaultReason::SENSOR);
            return;
        }
        if (_current >= TEMP_MAX_C) {
            enterFault(FaultReason::OVER_TEMP);
            return;
        }

        HeaterState prevState = _state;

        if (_target <= TEMP_SETPOINT_MIN_C) {
            _state = HeaterState::IDLE;
            _pid.reset();
            _pwmOutput = 0;
        } else {
            float dtSeconds = PID_SAMPLE_MS / 1000.0f;
            float output = _pid.compute(_target, _current, dtSeconds);
            // Belt and braces: PID.h already clamps to PID_OUTPUT_MAX_CAPPED
            // internally, but constrain against the same derated ceiling
            // here too so this line can never become the one place that
            // quietly lets full 255/100% duty back in.
            _pwmOutput = (uint8_t)constrain(output, PID_OUTPUT_MIN, PID_OUTPUT_MAX_CAPPED);

            bool atTemp = fabsf(_target - _current) <= TEMP_AT_TARGET_TOL_C;
            _state = atTemp ? HeaterState::AT_TEMP : HeaterState::HEATING;
        }

        // Runaway watchdog #1: while actively heating, require steady progress.
        if (_state == HeaterState::HEATING) {
            if (prevState != HeaterState::HEATING) {
                _watchWindowStartMs = now;
                _watchWindowStartTemp = _current;
            } else if (now - _watchWindowStartMs >= WATCH_PERIOD_MS) {
                if (_current - _watchWindowStartTemp < WATCH_INCREASE_C) {
                    enterFault(FaultReason::RUNAWAY_NO_RISE);
                    return;
                }
                _watchWindowStartMs = now;
                _watchWindowStartTemp = _current;
            }
        }

        // Runaway watchdog #2: once settled, don't allow sustained drift away
        // from target (e.g. heater stuck on, or thermistor slipped out).
        if (_state == HeaterState::AT_TEMP || prevState == HeaterState::AT_TEMP) {
            float deviation = fabsf(_target - _current);
            if (deviation > RUNAWAY_MAX_DEVIATION_C) {
                if (!_drifting) {
                    _drifting = true;
                    _driftStartMs = now;
                } else if (now - _driftStartMs >= RUNAWAY_TIMEOUT_MS) {
                    enterFault(FaultReason::RUNAWAY_DRIFT);
                    return;
                }
            } else {
                _drifting = false;
            }
        }

        _windowStartMs = now; // resync PWM window to the new duty cycle
    }

    applyOutput();
}

void Heater::applyOutput() {
    unsigned long now = millis();
    unsigned long elapsed = now - _windowStartMs;
    if (elapsed >= PID_SAMPLE_MS) {
        _windowStartMs = now;
        elapsed = 0;
    }
    unsigned long onTime = ((unsigned long)_pwmOutput * PID_SAMPLE_MS) / 255UL;
    digitalWrite(_pin, elapsed < onTime ? HIGH : LOW);
}
