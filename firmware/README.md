# FlowForge Firmware

Firmware for a hand-built plastic-bottle-extruder machine: Arduino Mega 2560
+ RAMPS 1.6, one hotend (thermistor + PID-controlled heater cartridge), one
E0 stepper (A4988) feeding the extruder screw/gear, an SSD1306 128x64 OLED,
and an HW-040 rotary encoder for input.

The temperature control and menu/display are deliberately modeled on
Marlin's approach (PID shape, thermal-runaway watchdogs, hierarchical
click-to-navigate menu, status-screen layout) collapsed down to what this
single-hotend, single-motor machine actually needs — this is a small
hand-rolled firmware, not a Marlin build.

## Safety-relevant behavior (read this first)

- **The machine boots idle.** On power-up the heater target is 0°C (room
  temperature) and the motor is disabled. Nothing heats or moves until the
  operator explicitly sets a target (Temperature menu or a preheat preset)
  and/or starts the motor. If you're used to an earlier build of this
  firmware that auto-heated to 200°C on boot, that behavior has been
  removed.
- **Cold-extrusion interlock.** The motor will not start (click on Home is
  ignored, with a short "no" beep) unless the hotend is at or above
  `TEMP_MIN_EXTRUDE_C` (170°C by default, `include/config.h`).
- **Thermal runaway watchdogs** (mirrors Marlin's approach): while actively
  heating, the temperature must rise by `WATCH_INCREASE_C` every
  `WATCH_PERIOD_MS`, or it's treated as a heater/thermistor fault. Once
  settled, if the reading drifts more than `RUNAWAY_MAX_DEVIATION_C` from
  target for longer than `RUNAWAY_TIMEOUT_MS`, that's also a fault (stuck
  heater / thermistor fell out). An open/shorted thermistor and exceeding
  `TEMP_MAX_C` (260°C) are also immediate faults.
- **A FAULT is latched.** The display shows a full-screen alert, the heater
  and motor are forced off, and the *only* way out is a power cycle — this
  is intentional, not a bug.
- **Long-press the encoder, from anywhere, to immediately stop the motor**
  (it does not touch the heater). This works even mid-menu.

## Hardware wiring (`include/pins.h`)

| Signal | Pin | Notes |
|---|---|---|
| E0 STEP | D26 | RAMPS E0 driver slot (A4988) |
| E0 DIR | D28 | |
| E0 ENABLE | D24 | active LOW (LOW = driver enabled) |
| Hotend heater | D10 | MOSFET output; software-PWM driven, see below |
| Hotend thermistor | A13 | RAMPS T0 analog input, 4.7kΩ series divider |
| OLED SDA/SCL | D20/D21 | hardware I2C (Wire), address `0x3C` |
| Encoder A / B | D35 / D37 | HW-040 quadrature, polled (not interrupt-driven) |
| Encoder button | D31 | active LOW |
| Buzzer | D33 | `tone()`, Timer2 |

The heater is driven by a slow **software PWM window** (not `analogWrite`)
because pin 10 shares Timer2 with the Arduino core's `tone()` used for the
buzzer — see the comment in `src/Heater.h` for details. The encoder is
**polled** every `loop()` iteration rather than interrupt-driven, because
its pins (PORTC on the Mega2560) don't support pin-change interrupts — see
`src/Encoder.h`.

## Build & flash

This is a [PlatformIO](https://platformio.org/) project.

```sh
pio run                 # build
pio run -t upload       # build + flash over USB
pio device monitor       # serial monitor (115200 baud) - only used for a
                          # boot-time warning if the OLED doesn't ACK on I2C
```

Libraries (`platformio.ini`): Adafruit SSD1306, Adafruit GFX, Adafruit
BusIO — PlatformIO fetches these automatically on first build.

## Menu map

```
Home (status screen)
 |-- rotate  -> Main Menu
 |-- click   -> start/stop the extruder motor (instant, only if hot enough)
 `-- long-press (from anywhere) -> immediate motor stop

Main Menu
 |-- Back           -> Home
 |-- Temperature     -> Temperature Menu
 |-- Motor Speed     -> Set Motor Speed (edit screen)
 `-- Settings        -> Settings Menu (read-only)

Temperature Menu
 |-- Back            -> Main Menu
 |-- Nozzle           -> Set Temperature (edit screen)
 |-- Preheat PET      -> sets target to PREHEAT_PET_C (250C) immediately
 |-- Preheat HDPE     -> sets target to PREHEAT_HDPE_C (200C) immediately
 `-- Cooldown         -> sets target to 0

Settings Menu (read-only calibration readout)
 |-- Back            -> Main Menu
 |-- Steps/mm         -> EXTRUDER_STEPS_PER_MM
 |-- Max speed        -> EXTRUDER_FEEDRATE_MAX_MM_S
 `-- Accel            -> EXTRUDER_ACCEL_MM_S2

Set Temperature (edit screen)
 |-- rotate  -> adjust target by TEMP_STEP_C, clamped [0, TEMP_MAX_C]
 `-- click   -> confirm, return to Temperature Menu

Set Motor Speed (edit screen)
 |-- rotate  -> adjust speed level, clamped [SPEED_LEVEL_MIN, SPEED_LEVEL_MAX]
 `-- click   -> confirm, return to Main Menu
```

Every menu's row 0 is always **Back**, one level up — the top-level Back
(Main Menu) returns to Home. Every screen but Home is reached only by
rotating into the menu; **Home's click always toggles the motor directly**
and never enters the menu — this is a deliberate shortcut for fast,
glove-friendly start/stop on a handheld device, kept even though it departs
from Marlin (which only starts jobs through a menu action).

## Motor calibration (steps/mm)

The extruder speed is expressed in real units (mm/s of filament fed),
computed from `EXTRUDER_STEPS_PER_MM` in `include/config.h`. The shipped
default (500 steps/mm) is a **placeholder** — it depends on your motor's
steps/rev, the A4988 microstepping set by the RAMPS jumpers, and your
gear/pulley ratio, none of which this firmware can know on its own.

To calibrate:
1. Go to **Main Menu → Motor Speed** and set a known level (level 5 is a
   good midpoint).
2. Start the motor from Home (heat up past `TEMP_MIN_EXTRUDE_C` first) and
   let it run for a fixed, measured time — e.g. exactly 60 seconds — then
   stop it.
3. Measure how much filament actually came out, in mm.
4. Compute the commanded distance for that run: `commanded_mm =
   levelToFeedrateMmS(level) * seconds_run` (with the defaults, level 5 ≈
   2.0mm/s, so 60s ≈ 120mm commanded).
5. Rescale: `new_steps_per_mm = EXTRUDER_STEPS_PER_MM * (commanded_mm /
   measured_mm)`. Update `EXTRUDER_STEPS_PER_MM` in `config.h` and reflash.

`EXTRUDER_FEEDRATE_MIN_MM_S`/`_MAX_MM_S` are deliberately conservative out
of the box — they were chosen to keep the step-interrupt rate identical to
what this firmware already ran at before this update (200–2000 steps/sec),
not Marlin's own much higher extruder defaults. Once steps/mm is calibrated
and you've confirmed the motor tracks cleanly at the current ceiling, you
can raise `EXTRUDER_FEEDRATE_MAX_MM_S` (and `EXTRUDER_ACCEL_MM_S2` if you
want a snappier ramp) — increase gradually and watch/listen for missed
steps or stalling at the top of the range before committing to a value.

## PID tuning

`include/config.h`'s `PID` section documents each constant inline
(`PID_KP`/`PID_KI`/`PID_KD`, `PID_FUNCTIONAL_RANGE_C`, `PID_K1`,
`PID_SAMPLE_MS`). The algorithm itself (`src/PID.h`) is a Marlin-style PID:
outside `PID_FUNCTIONAL_RANGE_C` of the target it just bangs the heater
full on/off (avoids integral windup while ramping from cold); inside that
band it runs proportional + anti-windup-clamped integral +
EMA-smoothed derivative-on-measurement (quieter than raw derivative, so a
single noisy thermistor ADC sample can't spike the output). The shipped
gains are a starting point for a typical salvaged RepRap-era hotend — retune
for your actual hotend's thermal mass/insulation if the response is too
sluggish or oscillates.

## Module map

| Module | Responsibility |
|---|---|
| `src/Thermistor.*` | Hotend temperature sensing |
| `src/PID.h` | Generic PID control loop |
| `src/Heater.*` | PID + thermistor + safety watchdogs → heater output |
| `src/Extruder.*` | Accel-ramped Timer1 stepper pulses for the E0 A4988 |
| `src/Encoder.*` | Polled HW-040 quadrature + button |
| `src/Buzzer.*` | Non-blocking tone sequencer |
| `src/Display.*` | SSD1306 rendering — boot logo, status screen, menus |
| `src/UI.*` | Menu state machine tying the above together |

Pin map: `include/pins.h`. Tunable constants: `include/config.h`.
