# FlowForge: An Open-Source 3D Printing Pen Built Entirely From Waste

**Pitch:** A handheld 3D pen made from a dead 3D printer, printing filament made from plastic bottles.

**Difficulty:** Intermediate
**Categories:** Sustainability · 3D Printing · Reuse
**Licence:** CERN-OHL-S v2 (hardware) · MIT (firmware)

> **Draft status.** Sections marked **[MEASURE]** require bench data before publication. Do not publish with placeholders in place — a jury checks numbers.

---

## 1. The problem

Two waste streams exist in every maker space, and they never meet.

The first is dead 3D printers. A printer is usually retired because of one failure — a warped bed, a fried mainboard, a seized lead screw, a cracked frame. The parts that took the most engineering to make are almost always still perfect. The hot end still holds temperature. The NEMA 17 steppers still turn. The extruder gears still grip. The PTFE tube is still slick. These parts get thrown away attached to a machine that failed somewhere else entirely.

The second is PET bottles. In Yaoundé they are everywhere, and they are chemically almost the same material as the PETG that makers pay import prices for.

Meanwhile, a commercial 3D printing pen is a closed product. You cannot see inside it, you cannot repair it, you cannot modify it, and when the nozzle clogs it becomes waste itself. A spool of imported filament costs **[MEASURE: local price of 1 kg virgin PLA/PETG in XAF]**, which puts casual 3D printing out of reach for most people here.

FlowForge exists because those two waste streams solve each other.

---

## 2. The idea: closing the loop

FlowForge is a handheld filament extrusion pen in which:

- the **machine** is recovered from e-waste, and
- the **feedstock** is recovered from plastic waste, and
- the **enclosure** is printed on that same recovered feedstock.

That last point is what makes this a loop rather than a gesture. The pen's own body is made of the material the pen consumes. A FlowForge can, in principle, print the parts for the next FlowForge.

```
   PET bottle
       │  wash · dry · strip-cut
       ▼
  rPET filament ──────────────┐
       │  extrude              │
       ▼                       │
    FlowForge ────► printed part
       ▲                       │
       │  printed enclosure    │
       └───────────────────────┘

  Dead 3D printer ──► hot end · nozzle · extruder · stepper · PTFE · controller
                              │
                              ▼
                          FlowForge
```

3D printing pens have existed for over a decade. A handful are hackable. Almost none are open source. As far as we can establish, none have been built from recovered material on both axes at once.

---

## 3. What is recycled, and from what

Every component below is either salvaged or printed from recycled feedstock. The "new equivalent" column exists so that a reader without a donor printer can still build this — that is the difference between a demonstration and a recipe.

| # | Part | Salvaged from | Condition / preparation | New equivalent | New cost (XAF) |
|---|---|---|---|---|---|
| 1 | Hot end (heater block, heat break, heatsink) | Donor FDM printer | Cold-pulled, thermistor and cartridge continuity checked | E3D V6 clone, 12 V | **[MEASURE]** |
| 2 | Nozzle, 0.4 mm brass | Donor FDM printer | Cleared, bore checked against drill shank | Brass 0.4 mm MK8 | **[MEASURE]** |
| 3 | Extruder mechanism | Donor FDM printer | Hobbed gear degreased, idler spring replaced | MK8 direct drive | **[MEASURE]** |
| 4 | Stepper motor, NEMA 17 | Donor FDM printer | Winding resistance checked, bench-run | NEMA 17 42×34, 1.5 A | **[MEASURE]** |
| 5 | PTFE tube, 2 × 4 mm | Donor FDM printer | Cut to length, ends faced square | PTFE 2/4 mm | **[MEASURE]** |
| 6 | Arduino Mega 2560 | Donor printer controller | Bootloader reflashed | Arduino Mega 2560 | **[MEASURE]** |
| 7 | RAMPS 1.6 shield + stepper driver | Donor printer controller | MOSFET and driver bench-tested | RAMPS 1.6 + DRV8825 | **[MEASURE]** |
| 8 | Thermistor, 100 kΩ NTC | Donor FDM printer | Resistance verified at ambient | 100 kΩ NTC B3950 | **[MEASURE]** |
| 9 | Cooling fan | Donor FDM printer | Bearing checked, blades cleaned | 12 V 40 mm axial | **[MEASURE]** |
| 10 | Filament, 1.75 mm rPET | PET drink bottles | Washed, dried, strip-cut, drawn | rPETG 1.75 mm spool | **[MEASURE]** |
| 11 | Enclosures (74 g) | Printed on rPET | See §5 print schedule | — | 7,417 |

**Salvaged part count: 9 of 11 line items.** Only fasteners, wiring and the bench supply are bought new.

### A note on reusing the controller

FlowForge runs on an Arduino Mega with a RAMPS 1.6 shield. For a device that drives one stepper and one heater, that is a large board. We chose it deliberately: it is the donor printer's own controller, and reusing it means **zero new silicon enters the project**. Buying a smaller microcontroller would have made a neater pen and a worse argument. A miniaturised variant is on the roadmap (§13).

---

## 4. System architecture

FlowForge is three units, not one object.

**Unit 1 — the pen.** Holds the hot end, nozzle and vented barrel inside a heat-isolated handle. Filament arrives through a PTFE guide; power and thermistor wiring arrive through the same tether. The pen contains no electronics beyond the heater cartridge, thermistor and fan.

**Unit 2 — the electronics unit.** A desktop wedge, 100 × 80 mm footprint, 50 mm at the tall face falling to 40 mm at the rear. It houses the Arduino Mega and RAMPS, the extruder stepper and its driver, the OLED display, the rotary encoder, the buzzer and the cooling control. Filament enters at the rear inlet, is gripped by the extruder, and leaves through the front outlet into the PTFE tether.

**Unit 3 — bench supply.** 12 V DC. Measured draw **[MEASURE: steady-state and peak current]**; design headroom taken at 1.67 A per the working notes, i.e. approximately 40 W.

### Why tethered

Putting the motor and controller on the desk rather than in the hand was a deliberate trade:

- The hand holds roughly 40 g instead of roughly 400 g.
- The stepper's heat and vibration are decoupled from the grip.
- The donor controller can be reused as-is rather than replaced with something small enough to hold.
- The thermal mass near the user's hand is limited to the hot end and its isolation.

The cost is a tether. For a first version aimed at reproducibility, that was the right trade.

```
 ┌──────────────┐   12 V    ┌────────────────────────┐
 │ Bench supply │──────────►│  Electronics unit      │
 └──────────────┘           │  ┌──────────────────┐  │
                            │  │ Arduino Mega     │  │
  rPET filament ───────────►│  │ + RAMPS 1.6      │  │
        (rear inlet)        │  └───┬────┬─────┬───┘  │
                            │      │    │     │      │
                            │  stepper OLED encoder  │
                            │      │                 │
                            │   extruder ──► outlet ─┼──┐
                            └────────────────────────┘  │
                                                        │ PTFE + wiring tether
                            ┌───────────────────────────┘
                            ▼
                     ┌─────────────┐
                     │   The pen   │  heater · thermistor · fan · nozzle
                     └─────────────┘
```

---

## 5. Mechanical design

### Hot end stack

The pen's thermal core is the donor hot end, measured and modelled as three stacked sections along the axis:

| Section | Diameter (mm) | Length (mm) | Function |
|---|---|---|---|
| A | 15.7 | 3.1 | Heat-break collar / mounting shoulder |
| B | 11.9 | 5.7 | Throat — narrowest section, thermal isolation |
| C | 15.9 | 3.7 | Heater block seat |

Section B is the critical dimension. It is the thermal break, and the barrel is designed so that no printed plastic touches it — the surrounding wall stands off with an air gap, because rPET enclosure material would soften long before the hot end reaches operating temperature.

### The pen body

The barrel is ribbed along its length. The ribs are not decoration: they increase surface area for convective cooling and reduce the contact patch between the hot section and the grip. Above the barrel sits the **hot end vent**, an open slot that lets the fan's airflow exit rather than recirculate.

The handle is a separate printed shell, hatched in the sketches, joined to the barrel at a break line that sits behind the heat break. The tether — PTFE guide plus wiring loom — enters at the rear through a single strain-relieved cabling inlet.

### Electronics unit body

The wedge form is functional. The tall front face carries the OLED and the encoder at a natural viewing angle; the low rear face carries the filament inlet, power switch, programming port and DC jack, keeping all cabling behind the unit. The sloped top surface will carry a printed surface pattern and the FlowForge badge.

Controls, front face: rotary encoder (replacing the increment / decrement / extrude button cluster from the earlier design), OLED display.
Rear face: filament inlet, power button, programming port, DC jack.

There is no separate extrude trigger. Start/stop is a single click of the encoder from the Home screen, and a long-press from anywhere is an immediate motor stop — see §7.

The current geometry in `stl/` is a **120 × 80 mm footprint**: the bottom frame body is 120 × 80 × 20 mm and the top frame body 120 × 80 × 43 mm, giving a ~63 mm overall stack. The earlier 100 × 80 × 50/40 mm envelope quoted in the working notes is superseded by these models.

### Print schedule

All parts printed on recycled rPET, colour: sturdy grey. The `stl/` directory now holds **fifteen** parts across two sub-assemblies. Solid volumes and bounding boxes below are computed directly from the STL meshes; masses are as-sliced.

**Electronics unit**

| Part | STL | Bounding box (mm) | Solid volume (cm³) | Mass (g) |
|---|---|---|---|---|
| Bottom frame body | `bottom frame body.stl` | 120.0 × 80.0 × 20.0 | 22.32 | 27.54 † |
| Top frame body | `top frame body.stl` | 120.0 × 80.0 × 43.0 | 33.22 | 41.71 |
| Top body design | `top body design.stl` | 69.8 × 39.8 × 0.4 | 1.11 | 1.37 |
| Rotary encoder knob | `rotatory encoder knob.stl` | 15.0 × 13.5 × 13.5 | 0.94 | 1.23 |
| FlowForge badge | `flowforge badge.stl` | 1.6 × 53.8 × 13.8 | 0.72 | 0.90 |
| Bottom support | `bottom support.stl` | 9.8 × 9.8 × 3.0 | 0.21 | 0.79 |
| Screen body design | `screen body design.stl` | 0.4 × 43.1 × 31.2 | 0.38 | 0.43 |
| "Made in Cameroon" plate | `made in cameroon.stl` | 37.8 × 1.2 × 7.8 | 0.23 | **[MEASURE]** |
| **Subtotal (first seven)** | | | | **73.97** |

† The `bottom frame body.stl` mesh has been revised since the 27.54 g figure was recorded. Re-slice before publishing.

**Pen assembly** — these parts postdate the original print schedule and have no recorded masses yet.

| Part | STL | Bounding box (mm) | Solid volume (cm³) | Mass (g) |
|---|---|---|---|---|
| Barrel / body cylinder | `body cylinder.stl` | ⌀22.0 × 90.0 | 9.70 | **[MEASURE]** |
| Handle | `handle.stl` | ⌀20.0 × 70.0 | 3.68 | **[MEASURE]** |
| Hot end cover shield | `hot end cover shield.stl` | ⌀22.0 × 38.6 | 2.72 | **[MEASURE]** |
| Hot end cover, half 1 | `hot end cover1.stl` | 10.1 × 20.2 × 21.6 | 1.36 | **[MEASURE]** |
| Hot end cover, half 2 | `hot end cover2.stl` | 10.1 × 20.2 × 21.6 | 1.36 | **[MEASURE]** |
| Top cap | `top cap.stl` | ⌀20.0 × 15.0 | 2.10 | **[MEASURE]** |
| Body1 (hot end cover variant) | `Body1.stl` | 10.1 × 20.3 × 29.0 | 1.80 | **[MEASURE]** |

`Body1.stl` is a taller variant of the split hot end cover and appears to be an in-progress revision of `hot end cover1/2`. **Decide which pair ships and remove the other before publication** — a reader cannot guess.

At **[MEASURE: confirm rate]** ≈ 100 XAF/g of recycled filament, the electronics-unit enclosure material cost ≈ **7,400 XAF**. The pen assembly adds roughly 22 cm³ of solid volume on top of that and must be re-costed once sliced.

> Note: an earlier revision of these figures doubled the bottom frame body (27.54 × 2) while keeping a 74.17 g total. The 73.97 g subtotal above is the reconciled version, and it now covers only the electronics unit. Verify the whole table against final slicer output before publishing.

---

## 6. Electronics

### Component list

| Function | Part | Notes |
|---|---|---|
| Controller | Arduino Mega 2560 | Salvaged from donor printer |
| Driver shield | RAMPS 1.6 | Salvaged |
| Stepper driver | A4988 | E0 socket, current-limited. Microstepping is set by the RAMPS jumpers, not in software |
| Extruder motor | NEMA 17 | E0 axis |
| Heater | Cartridge heater, 12 V | RAMPS D10, software-PWM driven |
| Temperature sense | 100 kΩ NTC thermistor, β 3950 | RAMPS T0 (A13), 4.7 kΩ series divider |
| Cooling | 40 mm axial fan, 12 V | Currently wired always-on |
| Display | SSD1306 OLED 128×64, I²C | SDA D20 / SCL D21, address `0x3C` |
| Input | HW-040 rotary encoder + push | Polled in software; see below |
| Audio feedback | Piezo buzzer | Alarm and confirmation tones, `tone()` on Timer2 |
| Power | 12 V bench supply | ≈ 40 W design headroom |

### Pin map (RAMPS 1.6 on Mega 2560)

Authoritative source: `firmware/include/pins.h`. This table is generated from it — if the two ever disagree, the header wins.

| Signal | Pin | Notes |
|---|---|---|
| E0 step | D26 | |
| E0 direction | D28 | |
| E0 enable | D12 | Active LOW (LOW = driver enabled) |
| Hotend heater | D10 | MOSFET output, slow software PWM |
| Thermistor T0 | A13 | RAMPS T0 analog input |
| OLED SDA / SCL | D20 / D21 | Hardware I²C via `Wire` |
| Encoder A / B | D35 / D37 | Quadrature, `INPUT_PULLUP` |
| Encoder switch | D31 | Active LOW |
| Buzzer | D33 | |

> **Corrected.** An earlier revision of this document listed encoder A/B on D31/D33, the switch on D35 and the buzzer on D37. Those assignments were rotated relative to the firmware and would not have worked. The table above matches `pins.h`.

Only the RAMPS signals FlowForge actually uses are listed. X/Y/Z drivers, the second heater, the bed output and all endstop inputs are unused. *Confirm against your board revision before wiring — RAMPS AUX pin assignments vary between clones.*

### Open item: fan control

The firmware defines **no fan pin and drives no fan output**. The 40 mm hot end fan is currently expected to run continuously from a 12 V rail. §9's safety guidance ("verify fan runs whenever heater is on") is therefore satisfied only by the wiring, not by any code, and §8's heat-creep failure mode has no firmware mitigation.

If the fan is to be controlled — the sensible version being "fan on whenever `Heater::state()` is not `IDLE`, plus a cooldown run-on" — it needs a pin in `pins.h` (RAMPS D9 is the conventional choice) and a hook in `Heater::update()`. Resolve this before publishing the build guide, one way or the other.

### Encoder decoding and debounce

The rotary encoder outputs are RC-filtered with small capacitors to ground (100 nF) on each channel. Mechanical encoders bounce badly, and software-only debouncing on a board this busy produced phantom counts during extrusion. Hardware filtering plus a state-machine decoder eliminated it.

The encoder is **polled, not interrupt-driven**. On the Mega 2560 pin-change interrupts only cover PORTB, PORTJ and PORTK; the encoder channels on D35/D37 sit on PORTC, which has none. This is the same reason Marlin polls its panel encoder on RAMPS builds. The consequence for the rest of the firmware is that nothing may block the main loop for long — which is why the heater uses a `millis()` PWM window, the buzzer is a non-blocking sequencer, and the display rate-limits its I²C flushes.

The button is debounced in software (`ENCODER_BUTTON_DEBOUNCE_MS`, 30 ms) and classified as a short click or a long-press at the `ENCODER_LONGPRESS_MS` threshold (600 ms).

### Stepper current

Set the driver reference voltage to match the salvaged motor, not the datasheet default. Measure the winding resistance, calculate the rated current, and set Vref accordingly for your driver (Vref = I × 0.5 for a DRV8825; for the A4988 assumed by the firmware comments, Vref = I × 8 × R_sense). Salvaged steppers are frequently not what their label claims.

### Timer allocation

Three peripherals contend for the Mega's timers, and the assignment is deliberate:

| Timer | Owner | Why |
|---|---|---|
| Timer1 | Extruder step generation | CTC mode; the ISR toggles the STEP pin at 2× the step frequency |
| Timer2 | `tone()` for the buzzer | Arduino core default on the Mega |
| — | Heater | Software PWM, no timer |

The heater deliberately does **not** use `analogWrite()`. Pin D10 is a Timer2 PWM pin, and Timer2 already belongs to `tone()`; driving both would have them fighting over the same timer. A hotend's thermal mass is seconds-long, so a `millis()`-driven on/off window at `PID_SAMPLE_MS` is both simpler and entirely precise enough.

---

## 7. Firmware

Written in C++ on PlatformIO, targeting `megaatmega2560` with the Arduino framework. Source lives in `firmware/`; `firmware/README.md` is the developer-facing reference and is authoritative for anything below.

The firmware does three things: hold temperature safely, drive the extruder on demand, and present a menu UI on the OLED. Its temperature control and menu structure are deliberately modelled on Marlin's approach — PID shape, thermal-runaway watchdogs, hierarchical click-to-navigate menus — collapsed to what a single-hotend, single-motor machine actually needs. It is a small hand-rolled firmware, not a Marlin build.

### Module map

| Module | Responsibility |
|---|---|
| `src/Thermistor.` | Oversampled ADC read, beta-equation conversion, open/short detection |
| `src/PID.h` | Generic PID loop, header-only |
| `src/Heater.` | PID + thermistor + safety watchdogs → heater output |
| `src/Extruder.` | Acceleration-ramped Timer1 step pulses for the E0 driver |
| `src/Encoder.` | Polled quadrature decode + debounced button |
| `src/Buzzer.` | Non-blocking tone sequencer |
| `src/Display.` | SSD1306 rendering — boot logo, status screen, menus, fault alert |
| `src/UI.` | Menu state machine tying the above together |

Pin map: `include/pins.h`. Every tunable constant: `include/config.h`. `main.cpp` does nothing but construct the objects and call `heater.update()`, `extruder.update()`, `ui.update()` in a tight non-blocking loop.

### Temperature control

`Heater` samples the thermistor, runs the PID and updates its state machine every `PID_SAMPLE_MS` (250 ms, i.e. **4 Hz** — not the 10 Hz quoted in earlier drafts). Between samples it does nothing but service the software PWM window.

Shipped gains, from `include/config.h`:

```cpp
#define PID_KP  22.0f
#define PID_KI   1.5f
#define PID_KD 110.0f
#define PID_SAMPLE_MS           250UL
#define PID_FUNCTIONAL_RANGE_C  15.0f   // outside this band: bang-bang
#define PID_K1                   0.95f  // derivative EMA smoothing
```

The loop itself (`src/PID.h`) is Marlin-shaped rather than textbook, in three specific ways:

- **Bang-bang bootstrap.** More than `PID_FUNCTIONAL_RANGE_C` below target, the heater simply goes full on and the PID state is reset; more than that above, full off. This avoids integral windup during the long ramp from cold, which a naive integrator would accumulate for minutes.
- **Accumulator-clamped anti-windup.** The clamp is applied to the raw pre-`Ki` accumulator (`±output_limit / Ki`), not to the output term, so the ceiling stays correct if you retune `Ki`.
- **EMA-smoothed derivative on measurement.** Derivative is taken on the measurement rather than the error, so a setpoint step produces no derivative kick, and it is smoothed with `PID_K1` so a single noisy ADC sample cannot spike the output.

These gains are a starting point for a typical salvaged RepRap-era hotend. Retune against your actual heater block and record the result — a used block with a loose cartridge behaves differently from a new one.

### Thermal runaway protection

This is the most important code in the project. A handheld device with a 260 °C heater and no supervision is dangerous. `Heater::update()` enforces **four** independent fault conditions, any one of which latches:

| Fault | Trigger | Constant |
|---|---|---|
| `SENSOR` | ADC pinned at either rail — thermistor open or shorted | ADC ≥ 1021 or ≤ 2 |
| `OVER_TEMP` | Measured temperature reaches the hard ceiling | `TEMP_MAX_C` = 260 °C |
| `RUNAWAY_NO_RISE` | While heating, temperature failed to climb enough in a window | `WATCH_INCREASE_C` = 2 °C per `WATCH_PERIOD_MS` = 20 s |
| `RUNAWAY_DRIFT` | After settling, reading drifted from target and stayed there | `RUNAWAY_MAX_DEVIATION_C` = 15 °C for `RUNAWAY_TIMEOUT_MS` = 10 s |

The sensor check catches the classic fire-starting failure: a thermistor that has fallen out of the block reads at the rail, and a naive firmware interprets that as "cold" and heats forever. `RUNAWAY_NO_RISE` catches a disconnected heater cartridge or a thermistor sitting in free air; `RUNAWAY_DRIFT` catches a heater MOSFET that has failed short.

A fault is **latched**. `enterFault()` forces the heater output low, the UI unconditionally stops the motor the instant it observes `HeaterState::FAULT`, the display switches to a full-screen alert, and the buzzer runs a repeating alarm. `Heater::setTarget()` becomes a no-op. The only way out is a power cycle. This is intentional.

Note one difference from the earlier draft's pseudocode: the fault path does **not** spin in an infinite loop. The main loop keeps running so the display stays rendered and the alarm keeps sounding — but the heater pin is re-driven low on every pass, belt and braces.

### Extrusion

Motion is expressed in real physical units, Marlin-style, rather than an abstract speed number:

```cpp
#define EXTRUDER_STEPS_PER_MM      500.0f   // placeholder — calibrate, see §10
#define EXTRUDER_FEEDRATE_MIN_MM_S   0.4f
#define EXTRUDER_FEEDRATE_MAX_MM_S   4.0f
#define EXTRUDER_ACCEL_MM_S2        30.0f
```

The UI exposes a discrete 1–10 speed level, mapped linearly onto that feedrate range. `setSpeedLevel()` and `setEnabled()` only set a *target*; `Extruder::update()`, called every loop iteration, ramps the actual step frequency toward it at `EXTRUDER_ACCEL_MM_S2`. That gives a soft start and a soft stop instead of an instant frequency jump, which reduces stall risk on a worn salvaged hobbed gear.

Stopping is ordered deliberately: the driver stays energised through the whole deceleration ramp and is only de-energised once the frequency reaches zero. De-energising mid-ramp would drop holding torque and let the motor coast and skip.

The cold-extrusion interlock is an **absolute temperature threshold**, not a proximity-to-setpoint test:

```cpp
bool Heater::safeToExtrude() const {
    return _state != HeaterState::FAULT && _current >= TEMP_MIN_EXTRUDE_C;  // 170 °C
}
```

Earlier drafts described a "within ±5 °C of setpoint" rule and a held trigger. Neither is what the firmware does. A click on the Home screen **toggles** the motor on or off; if the hotend is below `TEMP_MIN_EXTRUDE_C` the click is ignored and the buzzer gives a short "no" blip.

The feedrate ceiling is deliberately conservative — 0.4–4.0 mm/s keeps the step ISR in the same 200–2000 Hz envelope the firmware ran at before motion was expressed in real units. Raise it only after steps/mm is calibrated and the motor is confirmed to track cleanly at the top of the range.

### UI

A hierarchical menu state machine, driven entirely by one knob:

```
Home (status screen)
 |-- rotate     -> Main Menu
 |-- click      -> start/stop the extruder motor (only if hot enough)
 `-- long-press (from anywhere) -> immediate motor stop

Main Menu:        Back · Temperature · Motor Speed · Settings
Temperature Menu: Back · Nozzle · Preheat PET (250 °C) · Preheat HDPE (200 °C) · Cooldown
Settings Menu:    Back · Steps/mm · Max speed · Accel      (read-only readout)
Edit screens:     rotate adjusts and applies live, click confirms and returns
```

Row 0 of every menu is always **Back**, one level up. Home's click is the one deliberate departure from Marlin, where nothing starts without a menu action — on a handheld device a single-click start/stop is worth the inconsistency.

Three behaviours are worth calling out because they are safety-relevant:

- **The machine boots idle.** On power-up the heater target is 0 °C and the motor is disabled. Nothing heats until the operator sets a target through the Temperature menu or a preheat preset. An earlier build auto-heated to 200 °C on boot; that behaviour was removed.
- **Long-press stops the motor from anywhere**, including mid-menu. It does not touch the heater.
- **A latched fault overrides everything**, replacing the display with an alert screen and killing the motor regardless of what the UI was doing.

The display is decoupled from the state machine by a plain `UiModel` struct: `UI` fills in a snapshot each tick and `Display` only ever reads it. Rendering is rate-limited to `DISPLAY_REFRESH_MS` (150 ms) because a full-frame I²C flush takes several milliseconds and the encoder is polled in software.

### Preheat presets

`PREHEAT_PET_C` = 250 °C and `PREHEAT_HDPE_C` = 200 °C are one-click presets in the Temperature menu, sized for this machine's recycled-feedstock mix. Cooldown sets the target to 0. Reconcile the PET preset against the measured working window in §8 once you have it.

---

## 8. Working with rPET filament

This section is the honest core of the project. Recycled bottle PET is not a drop-in replacement for commercial filament, and pretending otherwise would waste the reader's time.

### Why it is difficult

**It is strongly hygroscopic.** PET absorbs atmospheric moisture quickly. Wet PET does not merely print poorly — the water flashes to steam in the melt zone, producing popping, foaming, voids and a weak, cloudy bead. In Yaoundé's humidity this happens within hours of drying.

**Diameter tolerance is poor.** Bottle-strip filament inherits every variation in bottle wall thickness plus every wobble in the cutting jig. Commercial filament holds ±0.02 mm. Strip-drawn rPET commonly does far worse, and the extruder's grip and the melt's volumetric flow both depend on that diameter being predictable.

**Bottle PET is not PETG.** Commercial PETG is glycol-modified specifically to suppress crystallisation. Raw bottle PET crystallises more readily, which makes it stiffer, more brittle, and prone to snapping inside the PTFE guide — where a break is genuinely annoying to clear.

### Protocol

1. **Wash and de-label.** Remove all adhesive residue. Glue chars in the melt zone and is a common clog source.
2. **Dry before cutting.** 65 °C for 4–6 hours.
3. **Strip-cut** to a width matched to your target diameter, then draw through a heated die.
4. **Dry again before use.** 65 °C, minimum 4 hours. Non-negotiable.
5. **Store with desiccant.** Assume it is wet if it has been out more than a few hours.
6. **Gauge before loading.** Measure diameter at several points along each length with calipers and reject outliers.

### Measured results

| Metric | Result |
|---|---|
| Mean diameter | **[MEASURE]** |
| Diameter spread (min–max over 1 m) | **[MEASURE]** |
| Working temperature window | **[MEASURE — expect roughly 230–250 °C]** |
| Successful extrusion runtime before jam | **[MEASURE]** |
| Bottles consumed per 100 g filament | **[MEASURE]** |

### Failure modes and fixes

| Symptom | Cause | Fix |
|---|---|---|
| Popping, bubbles, cloudy bead | Wet filament | Re-dry at 65 °C |
| Grinding at the hobbed gear | Diameter oversize, or extrusion attempted below temperature | Gauge filament; verify temperature interlock |
| Snap inside PTFE guide | Brittle over-crystallised PET | Cool the draw faster; shorten tether bend radius |
| Gradual under-extrusion then stop | Heat creep softening filament above the break | Verify fan runs whenever heater is on |
| Charred residue at nozzle | Label adhesive or contamination | Cold pull; improve washing step |

---

## 9. Safety

FlowForge operates a 260 °C-capable heater inside a hand-held enclosure. Treat it as a soldering iron that also moves.

- **Burn risk.** The nozzle and barrel reach operating temperature. The handle is thermally isolated by an air gap and a break line behind the heat break, but the barrel is not, and is not designed to be gripped.
- **Thermal runaway.** Mitigated in firmware by four independent latching checks — sensor fault, over-temperature ceiling, no-rise watchdog and post-settle drift watchdog (§7). Never disable them. Never run the pen unattended.
- **Sensor integrity.** A disconnected thermistor is the classic path to an uncontrolled heater. The rail-detection check catches it; verify this works by deliberately disconnecting the thermistor on the bench before first real use (§10, commissioning step 2).
- **Cold-extrusion interlock.** The motor will not start below 170 °C. This protects the mechanism, not the operator — do not treat it as a temperature guarantee.
- **Fan.** The hot end fan is not under firmware control (§6). If it is wired through a switch or a failing connector, nothing in the firmware will notice. Wire it directly to the 12 V rail.
- **Ventilation.** Molten PET releases fumes, and residual label adhesive or contamination releases worse. Work in ventilated space.
- **Electrical.** The bench supply is the only mains-connected element and stays on the desk. The tether carries 12 V only.
- **Children.** This is not a toy despite the form factor. Adult supervision required.

---

## 10. Build it yourself

### Print

Print all parts from §5 in rPET or PETG, sturdy grey — the eight electronics-unit parts and the pen assembly. Suggested settings — **[MEASURE: confirm against your results]**:

| Setting | Value |
|---|---|
| Nozzle | 120 °C |
| Bed | 75 °C |
| Layer height | 0.2 mm |
| Walls | 4 |
| Infill | 40 % (frame bodies), 20 % (cosmetic parts) |
| Cooling | 30 % |

The frame bodies are the long prints (27.5 g and 41.7 g). Print them first; the small parts are quick reprints if you revise.

### Assemble the pen

1. Seat the hot end so the heat break (section B, ⌀11.9) sits clear of the printed wall with its air gap intact.
2. Fit the nozzle, tighten *at temperature* to seal against the heater block.
3. Route the PTFE guide to butt squarely against the heat break — any gap here becomes a filament trap.
4. Mount the fan facing the heatsink fins, exhaust aligned with the hot end vent.
5. Route heater, thermistor and fan wiring through the rear inlet; strain-relieve before closing.
6. Close the handle shell.

### Assemble the electronics unit

1. Mount the Mega, seat the RAMPS shield, install the stepper driver — **check orientation twice**, backwards drivers die instantly.
2. Set driver Vref for your motor, and set the microstepping jumpers under the E0 slot (§6). The firmware cannot read either — whatever you choose here is what `EXTRUDER_STEPS_PER_MM` must be calibrated against.
3. Mount the extruder mechanism with its inlet and outlet aligned to the rear and front ports.
4. Wire heater to D10, thermistor to T0, stepper to E0, fan to a 12 V rail (see §6 — the fan is not firmware-controlled).
5. Mount OLED (I²C, D20/D21), encoder (A/B on D35/D37, switch on D31) and buzzer (D33) to the front face; fit the printed knob.
6. Fit the badge and close.

### Flash

```sh
cd firmware
pio run              # build
pio run -t upload    # build + flash over USB
pio device monitor   # 115200 baud — only used for a boot-time warning
                     # if the OLED does not ACK on I²C
```

PlatformIO fetches the three Adafruit libraries (SSD1306, GFX, BusIO) automatically on first build.

### Commission

1. **Power on with no filament.** The machine boots idle — target 0 °C, motor disabled — so nothing should heat or move. You should get the boot logo and chime. Confirm the status screen reads plausible room temperature; if it reads a rail value or faults immediately, stop and fix the thermistor wiring.
2. **Test the safety interlocks.** With the unit powered, disconnect the thermistor and confirm it faults with a sensor error, latches, sounds the alarm and refuses to do anything until a power cycle. Reconnect and power-cycle.
3. **Test the cold-extrusion interlock.** While still cold, click on the Home screen. The motor must not start; you should get a short "no" blip instead.
4. **Heat with no filament.** Temperature menu → Preheat PET (250 °C), or set a target manually. Confirm the fan runs, the temperature climbs steadily, the state reaches at-temperature and the PID does not oscillate more than ±2 °C. Retune the gains in `config.h` if it does.
5. **Calibrate steps-per-mm.** The firmware has no "extrude N mm" command, so calibration is timed, not commanded:
   - Main Menu → Motor Speed, set level 5 (≈ 2.0 mm/s with shipped defaults).
   - Heat past 170 °C, mark the filament at the inlet, start the motor from Home, run for exactly 60 s, stop.
   - Measure the filament actually consumed, in mm.
   - `commanded_mm = feedrate_mm_s × seconds` (60 s at level 5 ≈ 120 mm).
   - `new_steps_per_mm = EXTRUDER_STEPS_PER_MM × (commanded_mm / measured_mm)`.
   - Update `EXTRUDER_STEPS_PER_MM` in `config.h`, reflash, repeat until it converges. Salvaged hobbed gears are worn and the 500 shipped default is a placeholder.
6. **First draw.** Load gauged, dried rPET. Heat, wait for the at-temperature indication, and click once on the Home screen to start the motor. Adjust the speed level until the bead is continuous. Click again — or long-press from anywhere — to stop.

---

## 11. Results and impact

| Metric | Value |
|---|---|
| Salvaged components | 9 of 11 line items |
| E-waste diverted (mass of salvaged parts) | **[MEASURE]** |
| PET diverted per pen (enclosure) | 73.97 g |
| Bottles equivalent | **[MEASURE]** |
| Enclosure material cost | ≈ 7,400 XAF |
| Total build cost, salvaged route | **[MEASURE]** |
| Total build cost, all-new equivalent | **[MEASURE — sum §3 column]** |
| Commercial 3D pen + 1 kg virgin filament | **[MEASURE]** |
| Saving versus commercial route | **[MEASURE]** |
| Power draw at steady state | **[MEASURE]** |

The cost comparison is the number that matters. Fill it honestly — including the cases where salvage was *not* cheaper once time is counted.

---

## 12. What went wrong

*Fill from your actual build log. Candidate entries based on the risks identified during design:*

- **[LOG]** rPET moisture — how long between drying and unusable.
- **[LOG]** Diameter variation and what the extruder tolerated before slipping.
- **[LOG]** Filament snapping in the tether; bend radius that fixed it.
- **[LOG]** Encoder phantom counts before hardware debouncing.
- **[LOG]** Heat creep / thermal isolation iterations on the handle.
- **[LOG]** Anything that failed on the salvaged parts themselves.

Write this section properly. A reader learns more from it than from anything else in the document, and it is the section that most clearly separates a finished project from a rendered one.

---

## 13. Roadmap

- **Untethered V2** on a Xiao ESP32-C3, moving the controller into the hand and cutting the tether to filament-only.
- **Integrated filament production** — bring the bottle-to-filament step in-house so the loop is complete under one roof.
- **Desktop companion app** for temperature profiles per feedstock batch, since recycled material varies batch to batch.
- **Filament diameter sensor** with live flow compensation — the single change that would most improve rPET reliability.

---

## Attachments checklist

- [ ] **Schematics** — KiCad exports (no placeholders)
- [ ] **CAD** — STEP, STLs, Fusion 360 source
- [ ] **Code** — PlatformIO project, correct language tags, licence file
- [ ] **Cover image** — pen mid-draw, high resolution, no text
- [ ] **Build video** — design capture, slicing, print timelapse, assembly, electronics, demonstration
- [ ] **Exploded animation** — Fusion 360
- [ ] **BOM** — with provenance column (§3)

---

## Document maintenance

Sections §5 (print schedule), §6 (electronics) and §7 (firmware) are derived from files in this repository and go stale silently. When the code or the models change, re-check them against:

| Doc section | Source of truth |
|---|---|
| §5 print schedule | `stl/` and the slicer output |
| §5 enclosure envelope | `stl/bottom frame body.stl`, `stl/top frame body.stl` bounding boxes |
| §6 pin map | `firmware/include/pins.h` |
| §6 timer allocation | `firmware/src/Extruder.cpp`, `firmware/src/Heater.h` |
| §7 all constants | `firmware/include/config.h` |
| §7 menu map, §10 commissioning | `firmware/src/UI.cpp` and `firmware/README.md` |

Unresolved items flagged during the last sync:

1. **Fan control is undefined in firmware** (§6). Decide: wire it always-on and say so, or add a pin and a hook in `Heater::update()`.
2. **`Body1.stl` versus `hot end cover1/2.stl`** (§5) — two competing revisions of the same part are both committed. Pick one.
3. **`bottom frame body.stl` has been revised** since its 27.54 g mass was recorded (§5). Re-slice.
4. **Pen assembly masses are unmeasured** (§5), so the enclosure cost figure in §11 covers only the electronics unit.
5. **`made in cameroon.stl`** is not in the original print schedule and has no recorded mass.
