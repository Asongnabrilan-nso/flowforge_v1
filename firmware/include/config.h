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

// Marlin-style PID_MAX derating: cap the heater's ACTUAL output ceiling
// below the raw 0-255 PWM range so it is never driven at indefinite 100%
// duty (continuous full-rail current). The bang-bang bootstrap (PID.h) and
// the anti-windup clamp both key off this value, so capping it here caps
// both the cold-start current step and every sustained high-duty stretch
// (e.g. holding near TEMP_MAX_C, where the PID otherwise sits close to
// saturation for a long time). Salvaged RAMPS heater MOSFETs/screw
// terminals are the weak link here - this exists to keep them from cooking
// under sustained near-max current, not to protect the hotend itself.
// Lower further (e.g. 70) if your specific board still runs hot at 80%.
#define HEATER_MAX_DUTY_PCT   80
#define PID_OUTPUT_MAX_CAPPED ((uint8_t)((uint16_t)PID_OUTPUT_MAX * HEATER_MAX_DUTY_PCT / 100))

// |target - current| beyond this -> full-on/full-off bang mode instead of
// PID math (avoids integral windup while ramping from cold; mirrors
// Marlin's PID_FUNCTIONAL_RANGE).
#define PID_FUNCTIONAL_RANGE_C  15.0f
// Derivative EMA smoothing: weight retained from the previous sample
// (Marlin PID_K1). Higher = smoother/laggier, lower = snappier/noisier.
#define PID_K1                  0.95f

// ================= Temperature setpoint =================
// No default target - the heater boots idle at 0 (room temp) and only
// starts heating once the operator sets a target via the menu (manual edit
// or a preheat preset). See UI::begin().
#define TEMP_STEP_C            5.0f   // change per encoder detent
#define TEMP_SETPOINT_MIN_C    0.0f

// ================= Preheat presets =================
// One-click Temperature-menu presets for common recycled-bottle plastics
// (Marlin-style "Preheat PLA" buttons, collapsed to this machine's material mix).
#define PREHEAT_PET_C   250.0f
#define PREHEAT_HDPE_C  200.0f

// ================= Extruder motion =================
// Ported from Marlin's per-axis motion params (steps/mm, feedrate,
// acceleration), collapsed to FlowForge's single continuous-feed E0 motor.
//
// EXTRUDER_STEPS_PER_MM: retune once measured - mark 100mm of filament,
// extrude it at speed level 5, measure actual mm delivered, rescale this
// constant accordingly. Actual microstepping is set by the RAMPS A4988
// driver jumpers, not software - this constant must match whatever jumper
// config is physically installed.
#define EXTRUDER_STEPS_PER_MM      500.0f

// Feedrate range is deliberately kept at the SAME step-frequency envelope
// (200-2000Hz) the old abstract speed level already ran at - i.e.
// 0.4-4.0mm/s at the steps/mm above. Raise these only after steps/mm is
// calibrated and higher speeds are confirmed stable on real hardware
// (Marlin's own E-axis default of 25mm/s would drive this ISR ~6x harder).
#define EXTRUDER_FEEDRATE_MIN_MM_S   0.4f   // feedrate at SPEED_LEVEL_MIN
#define EXTRUDER_FEEDRATE_MAX_MM_S   4.0f   // feedrate at SPEED_LEVEL_MAX
#define EXTRUDER_ACCEL_MM_S2        30.0f   // soft-start/stop ramp rate
#define EXTRUDER_INVERT_DIR        false    // mirrors Marlin's INVERT_E0_DIR

// UI exposes a discrete 1-10 level, linearly mapped to the feedrate range
// above via Extruder::levelToFeedrateMmS().
#define SPEED_LEVEL_MIN     1
#define SPEED_LEVEL_MAX     10
#define SPEED_LEVEL_DEFAULT 5

// ================= Encoder =================
#define ENCODER_PULSES_PER_DETENT 4     // typical for HW-040
#define ENCODER_BUTTON_DEBOUNCE_MS 30
#define ENCODER_LONGPRESS_MS       600

// ================= Display =================
#define OLED_WIDTH      128
#define OLED_HEIGHT     64
#define OLED_I2C_ADDR   0x3C
#define DISPLAY_REFRESH_MS 150
