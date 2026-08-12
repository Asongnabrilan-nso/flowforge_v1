#include "Buzzer.h"

Buzzer buzzer;

void Buzzer::begin(uint8_t pin) {
    _pin = pin;
    pinMode(pin, OUTPUT);
    noTone(_pin);
}

void Buzzer::startSequence(const Note* notes, uint8_t count, bool loop) {
    _sequence = notes;
    _sequenceLen = count;
    _sequenceLoop = loop;
    _sequenceIdx = 0;
    _noteStartMs = millis();
    _playing = true;
    tone(_pin, _sequence[0].freqHz);
}

void Buzzer::click() {
    static const Note seq[] = { {2000, 25} };
    startSequence(seq, 1, false);
}

void Buzzer::bootChime() {
    // Blocking by design: only called once from setup() before loop() starts.
    static const Note seq[] = { {1200, 90}, {1600, 90}, {2200, 140} };
    for (auto& n : seq) {
        tone(_pin, n.freqHz);
        delay(n.durationMs);
        noTone(_pin);
        delay(20);
    }
}

void Buzzer::alarm(bool on) {
    if (on) {
        static const Note seq[] = { {2600, 150}, {0, 150} };
        startSequence(seq, 2, true);
    } else {
        _playing = false;
        _sequence = nullptr;
        noTone(_pin);
    }
}

void Buzzer::update() {
    if (!_playing || _sequence == nullptr) return;

    const Note& n = _sequence[_sequenceIdx];
    if (millis() - _noteStartMs >= n.durationMs) {
        _sequenceIdx++;
        if (_sequenceIdx >= _sequenceLen) {
            if (_sequenceLoop) {
                _sequenceIdx = 0;
            } else {
                _playing = false;
                noTone(_pin);
                return;
            }
        }
        _noteStartMs = millis();
        const Note& next = _sequence[_sequenceIdx];
        if (next.freqHz == 0) {
            noTone(_pin);
        } else {
            tone(_pin, next.freqHz);
        }
    }
}
