#pragma once
// Pin map for FlowForge on Arduino Mega 2560 + RAMPS 1.6.
// Only the signals FlowForge actually uses are defined here - the rest of
// the RAMPS wiring diagram (X/Y/Z, other heaters/fans, endstops) is unused.

// ---- Extruder stepper (E0 driver slot, A4988) ----
#define E0_STEP_PIN     26
#define E0_DIR_PIN      28
#define E0_ENABLE_PIN   24   // active LOW: LOW = driver enabled

// ---- Hotend ----
#define HEATER_0_PIN    10   // hotend cartridge heater MOSFET
#define TEMP_0_PIN      A13  // hotend thermistor (RAMPS T0 input)

// ---- OLED / encoder panel (TinyBoy2-style, RAMPS AUX-2/AUX-4 header) ----
// OLED is I2C on the Mega's hardware I2C bus (SDA=20, SCL=21) - the Wire
// library uses these automatically, no explicit pin defines needed.
#define BTN_EN1_PIN     35   // encoder quadrature A
#define BTN_EN2_PIN     37   // encoder quadrature B
#define BTN_ENC_PIN     31   // encoder push-button
#define BEEPER_PIN      33   // buzzer
