#pragma once
#include <Arduino.h>

// Reads the hotend NTC thermistor via the RAMPS analog divider
// (thermistor to GND, THERMISTOR_SERIES_OHMS pull-up to 5V, junction on
// TEMP_0_PIN) and converts to degrees C using the beta equation.
class Thermistor {
public:
    explicit Thermistor(uint8_t analogPin);

    // Blocking oversampled read + conversion. Cheap enough (a handful of
    // analogRead calls) to call once per PID sample period.
    float readCelsius();

    // True if the last reading indicated an open circuit (disconnected
    // thermistor) or a short - both are wiring/hardware faults, not
    // temperature extremes.
    bool lastReadFaulted() const { return _faulted; }

private:
    uint8_t _pin;
    bool _faulted = false;
};
