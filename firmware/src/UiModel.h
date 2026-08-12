#pragma once
#include <Arduino.h>
#include "Heater.h" // HeaterState, FaultReason

enum class Screen : uint8_t { HOME, TEMP, SPEED };

// Plain snapshot of everything the display needs to draw one frame. UI.cpp
// fills this in each tick; Display.cpp only ever reads it - keeps rendering
// logic decoupled from the menu state machine.
struct UiModel {
    Screen screen = Screen::HOME;
    bool editing = false;

    float currentTempC = 0.0f;
    float targetTempC = 0.0f;
    HeaterState heaterState = HeaterState::IDLE;
    FaultReason faultReason = FaultReason::NONE;

    uint8_t speedLevel = 1;
    bool extruding = false;
    bool safeToExtrude = false;
};
