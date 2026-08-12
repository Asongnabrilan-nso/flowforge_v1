#pragma once
// User-tunable constants for FlowForge. Grouped by subsystem so calibration
// doesn't require touching the driver code.

// ================= Thermistor =================
// Generic 100K NTC (beta ~3950), matches most salvaged RepRap-era hotends
// incl. JGAurora A5. Retune THERMISTOR_BETA if you have exact datasheet
// values - everything downstream (PID, safety limits) is in degrees C and
// doesn't care how the raw ADC was converted.
#define THERMISTOR_BETA        3950.0f
#define THERMISTOR_R0_OHMS     100000.0f  // resistance at THERMISTOR_T0_C
#define THERMISTOR_T0_C        25.0f
#define THERMISTOR_SERIES_OHMS 4700.0f    // RAMPS onboard divider resistor
#define THERMISTOR_OVERSAMPLE  8          // ADC samples averaged per reading

// ================= Safety =================
#define TEMP_MAX_C            260.0f  // absolute ceiling -> immediate fault
#define TEMP_MIN_EXTRUDE_C    170.0f  // cold-extrusion prevention threshold
#define TEMP_AT_TARGET_TOL_C  3.0f    // "reached target" tolerance

// Thermal runaway watchdog (same idea as Marlin's WATCH_TEMP_PERIOD): while
// heating, temperature must rise by at least WATCH_INCREASE_C every
// WATCH_PERIOD_MS, otherwise something is wrong (disconnected heater/
// thermistor, thermistor fell out of the block, etc).
#define WATCH_PERIOD_MS        20000UL
#define WATCH_INCREASE_C       2.0f

// Once at temperature, if the reading drifts away from target by more than
// this for longer than RUNAWAY_TIMEOUT_MS, latch a fault (stuck heater).
#define RUNAWAY_MAX_DEVIATION_C 15.0f
#define RUNAWAY_TIMEOUT_MS      10000UL

// ================= PID =================
// Starting values only - tune for your specific hotend mass/insulation.
#define PID_KP  22.0f
#define PID_KI  1.5f
#define PID_KD  110.0f
#define PID_OUTPUT_MIN   0
#define PID_OUTPUT_MAX   255
#define PID_SAMPLE_MS    250UL   // PID + heater window update period

// ================= Temperature setpoint =================
#define DEFAULT_TARGET_TEMP_C 200.0f
#define TEMP_STEP_C            5.0f   // change per encoder detent
#define TEMP_SETPOINT_MIN_C    0.0f

// ================= Extruder speed =================
// UI exposes an abstract 1-10 level (no mm/s calibration yet); this maps
// linearly to a step-pulse frequency. Retune once steps/mm is known.
#define SPEED_LEVEL_MIN     1
#define SPEED_LEVEL_MAX     10
#define SPEED_LEVEL_DEFAULT 5
#define STEP_FREQ_MIN_HZ    200
#define STEP_FREQ_MAX_HZ    2000

// ================= Encoder =================
#define ENCODER_PULSES_PER_DETENT 4     // typical for HW-040
#define ENCODER_BUTTON_DEBOUNCE_MS 30
#define ENCODER_LONGPRESS_MS       600

// ================= Display =================
#define OLED_WIDTH      128
#define OLED_HEIGHT     64
#define OLED_I2C_ADDR   0x3C
#define DISPLAY_REFRESH_MS 150
