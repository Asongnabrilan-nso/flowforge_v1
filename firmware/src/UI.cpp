#include "UI.h"
#include "../include/config.h"

UI ui;

// Menu row counts (row 0 is always "Back" in every menu below).
static const uint8_t kMenuMainCount     = 4; // Back, Temperature, Motor Speed, Settings
static const uint8_t kMenuTempCount     = 5; // Back, Nozzle, Preheat PET, Preheat HDPE, Cooldown
static const uint8_t kMenuSettingsCount = 4; // Back, Steps/mm, Max speed, Accel (read-only)

void UI::begin(Heater& heater) {
    _heater = &heater;
    // Boot idle: Heater::_target already default-initializes to 0.0f, so no
    // setTarget() call here - the heater stays at room temperature until the
    // operator sets a target via the Temperature menu or a preheat preset.
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
    _extrudeStartMs = millis();
    buzzer.click();
}

void UI::setPreheat(float celsius) {
    if (celsius < TEMP_SETPOINT_MIN_C) celsius = TEMP_SETPOINT_MIN_C;
    if (celsius > TEMP_MAX_C) celsius = TEMP_MAX_C;
    _model.targetTempC = celsius;
    _heater->setTarget(celsius);
    buzzer.click();
}

void UI::gotoMenu(Screen screen, uint8_t cursor, uint8_t itemCount) {
    _model.screen = screen;
    _menuCursor = cursor;
    _model.menuItemCount = itemCount;
}

uint8_t UI::moveCursor(uint8_t cursor, uint8_t count, int8_t rotation) {
    int16_t next = (int16_t)cursor + rotation;
    while (next < 0) next += count;
    return (uint8_t)(next % count);
}

void UI::handleHome(int8_t rotation, bool clicked) {
    if (rotation != 0) {
        gotoMenu(Screen::MENU_MAIN, 0, kMenuMainCount);
        return;
    }
    if (clicked) toggleExtrude();
}

void UI::handleMenuMain(int8_t rotation, bool clicked) {
    if (rotation != 0) {
        _menuCursor = moveCursor(_menuCursor, kMenuMainCount, rotation);
        return;
    }
    if (!clicked) return;

    switch (_menuCursor) {
        case 0: gotoMenu(Screen::HOME, 0, 0); buzzer.click(); break;
        case 1: gotoMenu(Screen::MENU_TEMP, 0, kMenuTempCount); buzzer.click(); break;
        case 2: gotoMenu(Screen::EDIT_SPEED, 0, 0); buzzer.click(); break;
        case 3: gotoMenu(Screen::MENU_SETTINGS, 0, kMenuSettingsCount); buzzer.click(); break;
    }
}

void UI::handleMenuTemp(int8_t rotation, bool clicked) {
    if (rotation != 0) {
        _menuCursor = moveCursor(_menuCursor, kMenuTempCount, rotation);
        return;
    }
    if (!clicked) return;

    switch (_menuCursor) {
        case 0: gotoMenu(Screen::MENU_MAIN, 0, kMenuMainCount); buzzer.click(); break;
        case 1: gotoMenu(Screen::EDIT_TEMP, 0, 0); buzzer.click(); break;
        case 2: setPreheat(PREHEAT_PET_C); break;
        case 3: setPreheat(PREHEAT_HDPE_C); break;
        case 4: setPreheat(0.0f); break;
    }
}

void UI::handleMenuSettings(int8_t rotation, bool clicked) {
    if (rotation != 0) {
        _menuCursor = moveCursor(_menuCursor, kMenuSettingsCount, rotation);
        return;
    }
    if (!clicked) return;

    if (_menuCursor == 0) {
        gotoMenu(Screen::MENU_MAIN, 0, kMenuMainCount);
        buzzer.click();
    }
    // Rows 1-3 are read-only calibration info (steps/mm, max speed, accel) -
    // no click behavior, no sound (nothing happened).
}

void UI::handleEditTemp(int8_t rotation, bool clicked) {
    if (rotation != 0) {
        float t = _model.targetTempC + rotation * TEMP_STEP_C;
        if (t < TEMP_SETPOINT_MIN_C) t = TEMP_SETPOINT_MIN_C;
        if (t > TEMP_MAX_C) t = TEMP_MAX_C;
        _model.targetTempC = t;
        _heater->setTarget(t);
    }
    if (clicked) {
        gotoMenu(Screen::MENU_TEMP, 1, kMenuTempCount);
        buzzer.click();
    }
}

void UI::handleEditSpeed(int8_t rotation, bool clicked) {
    if (rotation != 0) {
        int level = _model.speedLevel + rotation;
        if (level < SPEED_LEVEL_MIN) level = SPEED_LEVEL_MIN;
        if (level > SPEED_LEVEL_MAX) level = SPEED_LEVEL_MAX;
        _model.speedLevel = level;
        extruder.setSpeedLevel(level);
    }
    if (clicked) {
        gotoMenu(Screen::MENU_MAIN, 2, kMenuMainCount);
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
        } else {
            switch (_model.screen) {
                case Screen::HOME:          handleHome(rotation, clicked); break;
                case Screen::MENU_MAIN:     handleMenuMain(rotation, clicked); break;
                case Screen::MENU_TEMP:     handleMenuTemp(rotation, clicked); break;
                case Screen::MENU_SETTINGS: handleMenuSettings(rotation, clicked); break;
                case Screen::EDIT_TEMP:     handleEditTemp(rotation, clicked); break;
                case Screen::EDIT_SPEED:    handleEditSpeed(rotation, clicked); break;
            }
        }
    }

    _model.menuSelectedIndex = _menuCursor;
    _model.currentTempC = _heater->current();
    _model.targetTempC = _heater->target();
    _model.heaterState = _heater->state();
    _model.faultReason = _heater->faultReason();
    _model.safeToExtrude = _heater->safeToExtrude();
    _model.speedTargetMmS = Extruder::levelToFeedrateMmS(_model.speedLevel);
    _model.speedCurrentMmS = extruder.currentFeedrateMmS();
    _model.extrudeElapsedS = _model.extruding
        ? (uint16_t)((millis() - _extrudeStartMs) / 1000UL)
        : 0;

    buzzer.update();

    unsigned long now = millis();
    if (now - _lastRenderMs >= DISPLAY_REFRESH_MS) {
        _lastRenderMs = now;
        display.render(_model);
    }
}
