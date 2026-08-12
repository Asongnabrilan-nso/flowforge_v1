#include "Encoder.h"
#include "../include/pins.h"
#include "../include/config.h"

Encoder encoder;

// Quadrature transition matrix: index = (prevState<<2 | newState), each a
// 2-bit (A,B) reading. Yields -1/0/+1 per valid transition; invalid
// (skipped/bounced) transitions map to 0 so noise doesn't miscount.
static const int8_t QUAD_TABLE[16] = {
    0, -1, 1, 0,
    1, 0, 0, -1,
    -1, 0, 0, 1,
    0, 1, -1, 0
};

void Encoder::begin() {
    pinMode(BTN_EN1_PIN, INPUT_PULLUP);
    pinMode(BTN_EN2_PIN, INPUT_PULLUP);
    pinMode(BTN_ENC_PIN, INPUT_PULLUP);

    uint8_t a = digitalRead(BTN_EN1_PIN);
    uint8_t b = digitalRead(BTN_EN2_PIN);
    _prevQuadState = (a << 1) | b;
}

void Encoder::poll() {
    uint8_t a = digitalRead(BTN_EN1_PIN);
    uint8_t b = digitalRead(BTN_EN2_PIN);
    uint8_t newState = (a << 1) | b;

    uint8_t index = (_prevQuadState << 2) | newState;
    _pulseAccumulator += QUAD_TABLE[index];
    _prevQuadState = newState;

    if (_pulseAccumulator >= ENCODER_PULSES_PER_DETENT) {
        _pendingDetents++;
        _pulseAccumulator -= ENCODER_PULSES_PER_DETENT;
    } else if (_pulseAccumulator <= -ENCODER_PULSES_PER_DETENT) {
        _pendingDetents--;
        _pulseAccumulator += ENCODER_PULSES_PER_DETENT;
    }

    // --- Button: debounce, then classify as click (short) or long-press ---
    unsigned long now = millis();
    bool rawPressed = (digitalRead(BTN_ENC_PIN) == LOW); // active LOW

    if (rawPressed != _buttonPrevRaw) {
        _buttonPrevRaw = rawPressed;
        _buttonLastChangeMs = now;
    }

    if ((now - _buttonLastChangeMs) >= ENCODER_BUTTON_DEBOUNCE_MS &&
        rawPressed != _buttonStable) {
        _buttonStable = rawPressed;
        if (_buttonStable) {
            _buttonPressedAtMs = now;
            _longPressFired = false;
        } else if (!_longPressFired) {
            _clickedFlag = true;
        }
    }

    if (_buttonStable && !_longPressFired &&
        (now - _buttonPressedAtMs) >= ENCODER_LONGPRESS_MS) {
        _longPressFired = true;
        _longPressFlag = true;
    }
}

int8_t Encoder::takeRotation() {
    int8_t r = _pendingDetents;
    _pendingDetents = 0;
    return r;
}

bool Encoder::clicked() {
    bool r = _clickedFlag;
    _clickedFlag = false;
    return r;
}

bool Encoder::longPressed() {
    bool r = _longPressFlag;
    _longPressFlag = false;
    return r;
}
