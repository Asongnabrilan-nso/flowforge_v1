#pragma once
#include "UiModel.h"
#include "Encoder.h"
#include "Heater.h"
#include "Extruder.h"
#include "Buzzer.h"
#include "Display.h"

// Marlin-style menu navigation, adapted to this device's single knob:
//   - Home (status screen): a single click instantly starts/stops the
//     extruder motor (fast, glove-friendly shortcut - deliberately kept
//     even though it deviates from Marlin, where nothing starts without
//     going through a menu). Rotating from Home enters the Main Menu.
//   - Main Menu / Temperature / Settings: rotate moves a highlighted-row
//     cursor, click enters a submenu, fires a one-shot action (preheat
//     preset, cooldown), or drills into a dedicated edit screen. Row 0 of
//     every menu is always "Back", returning one level up.
//   - Edit screens (Nozzle temperature, Motor speed): rotate live-adjusts
//     the value and applies it immediately; click confirms and returns to
//     the parent menu, cursor restored to the row that was drilled into.
//   - Long-press, from anywhere, is an immediate motor stop (safety
//     shortcut) - it does not touch the heater.
//   - A latched heater FAULT overrides the display with an alert screen;
//     the only way out is a power cycle (see Heater.h).
//   - Boot-idle: the heater target starts at 0 (Heater::_target's default)
//     and nothing heats until the operator sets a target via the menu.
class UI {
public:
    void begin(Heater& heater); // Heater isn't a global singleton (it wraps
                                 // a Thermistor reference owned by main.cpp)
    void update(); // call every loop()

private:
    void handleHome(int8_t rotation, bool clicked);
    void handleMenuMain(int8_t rotation, bool clicked);
    void handleMenuTemp(int8_t rotation, bool clicked);
    void handleMenuSettings(int8_t rotation, bool clicked);
    void handleEditTemp(int8_t rotation, bool clicked);
    void handleEditSpeed(int8_t rotation, bool clicked);

    void toggleExtrude();
    void setPreheat(float celsius);
    void gotoMenu(Screen screen, uint8_t cursor, uint8_t itemCount);
    static uint8_t moveCursor(uint8_t cursor, uint8_t count, int8_t rotation);

    Heater* _heater = nullptr;
    UiModel _model;
    uint8_t _menuCursor = 0;
    unsigned long _extrudeStartMs = 0;
    unsigned long _lastRenderMs = 0;
};

extern UI ui;
