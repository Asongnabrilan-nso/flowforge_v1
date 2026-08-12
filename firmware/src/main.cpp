// FlowForge firmware - Arduino Mega 2560 + RAMPS 1.6
// Hand-held 3D printing pen built from recycled materials.
//
// Module map:
//   Thermistor  - hotend temperature sensing (src/Thermistor.*)
//   PID         - generic PID control loop (src/PID.h)
//   Heater      - PID + thermistor + safety watchdogs -> heater output (src/Heater.*)
//   Extruder    - Timer1-driven stepper pulses for the E0 A4988 (src/Extruder.*)
//   Encoder     - polled HW-040 quadrature + button (src/Encoder.*)
//   Buzzer      - non-blocking tone sequencer (src/Buzzer.*)
//   Display     - SSD1306 rendering, boot logo + UI screens (src/Display.*)
//   UI          - menu state machine tying the above together (src/UI.*)
//
// Pin map: include/pins.h   Tunable constants: include/config.h

#include <Arduino.h>
#include "../include/pins.h"
#include "../include/config.h"
#include "Thermistor.h"
#include "Heater.h"
#include "Extruder.h"
#include "Encoder.h"
#include "Buzzer.h"
#include "Display.h"
#include "UI.h"

Thermistor hotendThermistor(TEMP_0_PIN);
Heater hotendHeater(HEATER_0_PIN, hotendThermistor);

void setup() {
    buzzer.begin(BEEPER_PIN);
    encoder.begin();
    extruder.begin();
    hotendHeater.begin();

    bool oledOk = display.begin();
    display.showBootScreen();
    buzzer.bootChime();

    if (!oledOk) {
        // Nothing more we can show the user - fall back to a serial hint in
        // case a debug console is attached.
        Serial.begin(115200);
        Serial.println(F("SSD1306 not found at configured I2C address"));
    }

    ui.begin(hotendHeater);
}

void loop() {
    hotendHeater.update();
    ui.update();
}
