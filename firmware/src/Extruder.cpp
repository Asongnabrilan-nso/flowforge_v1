#include "Extruder.h"
#include "../include/pins.h"
#include "../include/config.h"

Extruder extruder;

static const uint32_t TIMER1_PRESCALER = 8;

static uint16_t freqToOcr1a(uint16_t stepFreqHz) {
    // ISR toggles the STEP pin, so it must run at 2x the desired step
    // (rising-edge) frequency.
    uint32_t ocr = (F_CPU / (TIMER1_PRESCALER * 2UL * stepFreqHz)) - 1;
    if (ocr > 65535UL) ocr = 65535UL;
    return (uint16_t)ocr;
}

float Extruder::levelToFeedrateMmS(uint8_t level) {
    if (level < SPEED_LEVEL_MIN) level = SPEED_LEVEL_MIN;
    if (level > SPEED_LEVEL_MAX) level = SPEED_LEVEL_MAX;
    float span = EXTRUDER_FEEDRATE_MAX_MM_S - EXTRUDER_FEEDRATE_MIN_MM_S;
    float steps = SPEED_LEVEL_MAX - SPEED_LEVEL_MIN;
    return EXTRUDER_FEEDRATE_MIN_MM_S + (level - SPEED_LEVEL_MIN) * span / steps;
}

void Extruder::begin() {
    pinMode(E0_STEP_PIN, OUTPUT);
    pinMode(E0_DIR_PIN, OUTPUT);
    pinMode(E0_ENABLE_PIN, OUTPUT);
    digitalWrite(E0_STEP_PIN, LOW);
    digitalWrite(E0_ENABLE_PIN, HIGH); // active LOW - start disabled
    setDirection(true);

    noInterrupts();
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;
    TCCR1B |= (1 << WGM12) | (1 << CS11); // CTC mode, prescaler /8
    TIMSK1 &= ~(1 << OCIE1A);             // interrupt stays off until update() ramps up
    interrupts();

    _level = SPEED_LEVEL_DEFAULT;
    _enabled = false;
    _currentFreqHz = 0.0f;
    _lastUpdateMs = millis();
    _driverEnabledPinLow = false;
}

void Extruder::setSpeedLevel(uint8_t level) {
    if (level < SPEED_LEVEL_MIN) level = SPEED_LEVEL_MIN;
    if (level > SPEED_LEVEL_MAX) level = SPEED_LEVEL_MAX;
    _level = level; // update() picks up the new target next tick
}

void Extruder::setDirection(bool forward) {
    _forward = forward;
    bool pinHigh = EXTRUDER_INVERT_DIR ? !forward : forward;
    digitalWrite(E0_DIR_PIN, pinHigh ? HIGH : LOW);
}

void Extruder::setEnabled(bool enabled) {
    _enabled = enabled;
    if (enabled && !_driverEnabledPinLow) {
        digitalWrite(E0_ENABLE_PIN, LOW); // power the driver now; update() ramps up the speed
        _driverEnabledPinLow = true;
    }
    // On disable: the driver stays powered (LOW) until update() ramps
    // _currentFreqHz down to 0 - applyFreq() cuts E0_ENABLE_PIN HIGH itself
    // at that point. De-energizing instantly mid-deceleration would drop
    // holding torque and let the motor coast/skip; ramping down while still
    // enabled gives a controlled stop, symmetric with the soft-start.
}

void Extruder::update() {
    unsigned long now = millis();
    unsigned long dtMs = now - _lastUpdateMs;
    if (dtMs == 0) return;
    _lastUpdateMs = now;
    float dtSeconds = dtMs / 1000.0f;

    float targetHz = _enabled ? levelToFeedrateMmS(_level) * EXTRUDER_STEPS_PER_MM : 0.0f;
    float maxDeltaHz = EXTRUDER_ACCEL_MM_S2 * EXTRUDER_STEPS_PER_MM * dtSeconds;

    if (_currentFreqHz < targetHz) {
        _currentFreqHz = min(_currentFreqHz + maxDeltaHz, targetHz);
    } else if (_currentFreqHz > targetHz) {
        _currentFreqHz = max(_currentFreqHz - maxDeltaHz, targetHz);
    }

    applyFreq(_currentFreqHz);
}

float Extruder::currentFeedrateMmS() const {
    return _currentFreqHz / EXTRUDER_STEPS_PER_MM;
}

void Extruder::applyFreq(float hz) {
    if (hz < 1.0f) { // effectively stopped - avoid freqToOcr1a(0) divide-by-zero
        TIMSK1 &= ~(1 << OCIE1A);
        digitalWrite(E0_STEP_PIN, LOW);
        if (!_enabled && _driverEnabledPinLow) {
            digitalWrite(E0_ENABLE_PIN, HIGH); // active LOW - fully de-energize now
            _driverEnabledPinLow = false;
        }
        return;
    }

    uint16_t ocr = freqToOcr1a((uint16_t)hz);
    noInterrupts();
    OCR1A = ocr;
    if (TCNT1 > ocr) TCNT1 = 0;
    interrupts();
    if (!(TIMSK1 & (1 << OCIE1A))) {
        TCNT1 = 0;
        TIMSK1 |= (1 << OCIE1A);
    }
}

ISR(TIMER1_COMPA_vect) {
    static bool stepState = false;
    stepState = !stepState;
    digitalWrite(E0_STEP_PIN, stepState ? HIGH : LOW);
}
