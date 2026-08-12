#pragma once
#include <Arduino.h>
#include "Heater.h" // HeaterState, FaultReason

enum class Screen : uint8_t {
    HOME,
    MENU_MAIN,
    MENU_TEMP,
    MENU_SETTINGS,
    EDIT_TEMP,
    EDIT_SPEED,
};

// Plain snapshot of everything the display needs to draw one frame. UI.cpp
// fills this in each tick; Display.cpp only ever reads it - keeps rendering
// logic decoupled from the menu state machine. Menu item labels/values are
// display-only content, hand-coded per-menu directly in Display.cpp (only 3
// menus exist); this struct carries just enough (menuSelectedIndex,
// menuItemCount) for Display to know which row to highlight and how many
// rows to scroll through.
struct UiModel {
    Screen screen = Screen::HOME;

    float currentTempC = 0.0f;
    float targetTempC = 0.0f;
    HeaterState heaterState = HeaterState::IDLE;
    FaultReason faultReason = FaultReason::NONE;

    uint8_t speedLevel = 1;
    float speedTargetMmS = 0.0f;   // Extruder::levelToFeedrateMmS(speedLevel)
    float speedCurrentMmS = 0.0f;  // extruder.currentFeedrateMmS(), actual ramped speed
    bool extruding = false;
    bool safeToExtrude = false;
    uint16_t extrudeElapsedS = 0;  // seconds since the current extrude session began

    uint8_t menuSelectedIndex = 0; // highlighted row within the current MENU_* screen
    uint8_t menuItemCount = 0;     // total rows (Back included) in the current MENU_* screen
};
