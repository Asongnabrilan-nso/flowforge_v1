#include "UI.h"
#include "../include/config.h"

UI ui;

void UI::begin(Heater& heater) {
    _heater = &heater;
    _model.targetTempC = DEFAULT_TARGET_TEMP_C;
    _heater->setTarget(DEFAULT_TARGET_TEMP_C);
    _model.speedLevel = SPEED_LEVEL_DEFAULT;
    extruder.setSpeedLevel(SPEED_LEVEL_DEFAULT);
}

void UI::toggleExtrude() {
    if (_model.extruding) {
        extruder.setEnabled(false);
        _model.extruding = false;
        buzzer.click();
        return;
    }

    if (!_heater->safeToExtrude()) {
        buzzer.click(); // short blip = "no" feedback; too cold to run the motor
        return;
    }

    extruder.setEnabled(true);
    _model.extruding = true;
    buzzer.click();
}

void UI::handleNavigate(int8_t rotation, bool clicked) {
    if (rotation != 0) {
        uint8_t next = (static_cast<uint8_t>(_model.screen) +
                        (rotation > 0 ? 1 : 2)) % 3; // +2 == -1 mod 3
        _model.screen = static_cast<Screen>(next);
    }

    if (clicked) {
        if (_model.screen == Screen::HOME) {
            toggleExtrude();
        } else {
            _editing = true;
            buzzer.click();
        }
    }
}

void UI::handleEdit(int8_t rotation, bool clicked) {
    if (rotation != 0) {
        if (_model.screen == Screen::TEMP) {
            float t = _model.targetTempC + rotation * TEMP_STEP_C;
            if (t < TEMP_SETPOINT_MIN_C) t = TEMP_SETPOINT_MIN_C;
            if (t > TEMP_MAX_C) t = TEMP_MAX_C;
            _model.targetTempC = t;
            _heater->setTarget(t);
        } else if (_model.screen == Screen::SPEED) {
            int level = _model.speedLevel + rotation;
            if (level < SPEED_LEVEL_MIN) level = SPEED_LEVEL_MIN;
            if (level > SPEED_LEVEL_MAX) level = SPEED_LEVEL_MAX;
            _model.speedLevel = level;
            extruder.setSpeedLevel(level);
        }
    }

    if (clicked) {
        _editing = false;
        buzzer.click();
    }
}

void UI::update() {
    encoder.poll();

    if (_heater->state() == HeaterState::FAULT) {
        // Safety: unconditionally stop the motor the instant a fault
        // latches, regardless of what the UI/encoder is doing.
        if (extruder.isEnabled()) extruder.setEnabled(false);
        _model.extruding = false;
        buzzer.alarm(true);
    } else {
        int8_t rotation = encoder.takeRotation();
        bool clicked = encoder.clicked();

        if (encoder.longPressed()) {
            if (_model.extruding) {
                extruder.setEnabled(false);
                _model.extruding = false;
                buzzer.click();
            }
        } else if (_editing) {
            handleEdit(rotation, clicked);
        } else {
            handleNavigate(rotation, clicked);
        }
    }

    _model.editing = _editing;
    _model.currentTempC = _heater->current();
    _model.targetTempC = _heater->target();
    _model.heaterState = _heater->state();
    _model.faultReason = _heater->faultReason();
    _model.safeToExtrude = _heater->safeToExtrude();

    buzzer.update();

    unsigned long now = millis();
    if (now - _lastRenderMs >= DISPLAY_REFRESH_MS) {
        _lastRenderMs = now;
        display.render(_model);
    }
}
