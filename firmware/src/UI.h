#pragma once
#include "UiModel.h"
#include "Encoder.h"
#include "Heater.h"
#include "Extruder.h"
#include "Buzzer.h"
#include "Display.h"

// Menu behaviour (see project notes for the reasoning):
//   - Rotate on any screen in "navigate" mode cycles HOME -> TEMP -> SPEED -> HOME.
//   - Click on TEMP/SPEED enters "edit" mode; rotate then adjusts the value;
//     click again confirms and returns to navigate mode.
//   - Click on HOME toggles extrusion on/off directly (blocked while too
//     cold - see TEMP_MIN_EXTRUDE_C).
//   - Long-press, from anywhere, is an immediate motor stop (safety
//     shortcut) - it does not touch the heater.
//   - A latched heater FAULT overrides the display with an alert screen;
//     the only way out is a power cycle (see Heater.h).
class UI {
public:
    void begin(Heater& heater); // Heater isn't a global singleton (it wraps
                                 // a Thermistor reference owned by main.cpp)
    void update(); // call every loop()

private:
    void handleNavigate(int8_t rotation, bool clicked);
    void handleEdit(int8_t rotation, bool clicked);
    void toggleExtrude();

    Heater* _heater = nullptr;
    UiModel _model;
    bool _editing = false;
    unsigned long _lastRenderMs = 0;
};

extern UI ui;
