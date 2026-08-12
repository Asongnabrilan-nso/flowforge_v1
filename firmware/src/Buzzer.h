#pragma once
#include <Arduino.h>

// Wraps tone()/noTone() with a tiny non-blocking sequencer so alarms and
// click feedback don't stall the main loop (needed because the encoder is
// polled in software - see Encoder.h).
class Buzzer {
public:
    void begin(uint8_t pin);
    void update(); // call every loop()

    void click();          // short blip, menu navigation/confirm feedback
    void bootChime();      // blocking - only intended for use during setup()
    void alarm(bool on);   // start/stop a repeating fault alarm

private:
    struct Note { uint16_t freqHz; uint16_t durationMs; };

    void startSequence(const Note* notes, uint8_t count, bool loop);

    uint8_t _pin = 0;
    const Note* _sequence = nullptr;
    uint8_t _sequenceLen = 0;
    uint8_t _sequenceIdx = 0;
    bool _sequenceLoop = false;
    unsigned long _noteStartMs = 0;
    bool _playing = false;
};

extern Buzzer buzzer;
