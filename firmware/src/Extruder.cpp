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

static uint16_t levelToFreqHz(uint8_t level) {
    if (level < SPEED_LEVEL_MIN) level = SPEED_LEVEL_MIN;
    if (level > SPEED_LEVEL_MAX) level = SPEED_LEVEL_MAX;
    uint32_t span = STEP_FREQ_MAX_HZ - STEP_FREQ_MIN_HZ;
    uint32_t steps = SPEED_LEVEL_MAX - SPEED_LEVEL_MIN;
    return STEP_FREQ_MIN_HZ + (uint32_t)(level - SPEED_LEVEL_MIN) * span / steps;
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
    OCR1A = freqToOcr1a(levelToFreqHz(SPEED_LEVEL_DEFAULT));
    TCCR1B |= (1 << WGM12) | (1 << CS11); // CTC mode, prescaler /8
    TIMSK1 &= ~(1 << OCIE1A);             // interrupt stays off until enabled
    interrupts();

    setSpeedLevel(SPEED_LEVEL_DEFAULT);
}

void Extruder::setSpeedLevel(uint8_t level) {
    if (level < SPEED_LEVEL_MIN) level = SPEED_LEVEL_MIN;
    if (level > SPEED_LEVEL_MAX) level = SPEED_LEVEL_MAX;
    _level = level;

    uint16_t ocr = freqToOcr1a(levelToFreqHz(level));
    noInterrupts();
    OCR1A = ocr;
    if (TCNT1 > ocr) TCNT1 = 0;
    interrupts();
}

void Extruder::setDirection(bool forward) {
    _forward = forward;
    digitalWrite(E0_DIR_PIN, forward ? HIGH : LOW);
}

void Extruder::setEnabled(bool enabled) {
    _enabled = enabled;
    digitalWrite(E0_ENABLE_PIN, enabled ? LOW : HIGH); // active LOW
    if (enabled) {
        noInterrupts();
        TCNT1 = 0;
        TIMSK1 |= (1 << OCIE1A);
        interrupts();
    } else {
        TIMSK1 &= ~(1 << OCIE1A);
        digitalWrite(E0_STEP_PIN, LOW);
    }
}

ISR(TIMER1_COMPA_vect) {
    static bool stepState = false;
    stepState = !stepState;
    digitalWrite(E0_STEP_PIN, stepState ? HIGH : LOW);
}
