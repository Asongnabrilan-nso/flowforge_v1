#pragma once
#include <Arduino.h>

// HW-040 quadrature encoder + push button, polled (not interrupt-driven).
//
// Why polled: on the Mega2560, pin-change interrupts only cover PORTB,
// PORTJ and PORTK. BTN_EN1/BTN_EN2 (35/37) live on PORTC, which has none -
// the same reason Marlin's RAMPS+panel builds poll these pins from the main
// loop instead of attaching interrupts. Call poll() as the first thing in
// loop(), every iteration, for reliable detent detection; avoid long
// blocking calls elsewhere (the display driver rate-limits its I2C writes
// for exactly this reason).
class Encoder {
public:
    void begin();

    // Call every loop() iteration. Cheap (a few digitalReads).
    void poll();

    // Detent delta accumulated since the last call, then cleared: -1, 0, +1
    // (occasionally +/-2 if polling was briefly starved).
    int8_t takeRotation();

    bool clicked();     // true once per short press-release
    bool longPressed(); // true once per press held >= ENCODER_LONGPRESS_MS

private:
    uint8_t _prevQuadState = 0;
    int8_t _pulseAccumulator = 0;
    int8_t _pendingDetents = 0;

    bool _buttonPrevRaw = false;
    bool _buttonStable = false;
    unsigned long _buttonLastChangeMs = 0;
    unsigned long _buttonPressedAtMs = 0;
    bool _longPressFired = false;

    bool _clickedFlag = false;
    bool _longPressFlag = false;
};

extern Encoder encoder;
