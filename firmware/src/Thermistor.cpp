#include "Thermistor.h"
#include <math.h>
#include "../include/config.h"

Thermistor::Thermistor(uint8_t analogPin) : _pin(analogPin) {}

float Thermistor::readCelsius() {
    uint32_t sum = 0;
    for (uint8_t i = 0; i < THERMISTOR_OVERSAMPLE; i++) {
        sum += analogRead(_pin);
    }
    float adc = (float)sum / THERMISTOR_OVERSAMPLE;

    // Open circuit (thermistor unplugged) or short circuit - both leave the
    // reading pinned near the rails instead of settling to a real value.
    if (adc >= 1021.0f || adc <= 2.0f) {
        _faulted = true;
        return NAN;
    }
    _faulted = false;

    float rNtc = THERMISTOR_SERIES_OHMS * adc / (1023.0f - adc);
    float t0Kelvin = THERMISTOR_T0_C + 273.15f;
    float kelvin = 1.0f / (1.0f / t0Kelvin + logf(rNtc / THERMISTOR_R0_OHMS) / THERMISTOR_BETA);
    return kelvin - 273.15f;
}
