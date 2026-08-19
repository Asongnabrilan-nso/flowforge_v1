# FlowForge — Abstract & Documentation Plan

*Draft v0.1 — for the TME "Master of Zero Waste" entry and parallel Hackster publication*

---

## 1. Abstract (~1,500 characters)

3D printing pens have existed for over a decade, but almost none are open source, and none are built from waste. FlowForge is both.

FlowForge is a handheld filament extrusion pen assembled from parts rescued from a decommissioned FDM 3D printer — hot end, nozzle, extruder mechanism, NEMA 17 stepper and PTFE tube — and fed with filament drawn from discarded PET bottles. Its enclosures are 3D printed, and every structural part is designed to print on the same recycled filament the pen itself consumes. The result is a closed material loop: waste plastic becomes a tool that draws in waste plastic.

The system has three units. A tethered pen carries the hot end, nozzle and vented barrel in a heat-isolated handle. A desktop electronics unit houses the extruder motor, driver, thermistor sensing, cooling fan, OLED display and rotary encoder, and feeds filament to the pen through a PTFE guide. A bench supply provides 24 V. Firmware runs closed-loop PID temperature control with thermal-runaway protection and user-set extrusion speed.

FlowForge is released fully open: Fusion 360 sources and STLs, KiCad schematics, a provenance-annotated BOM listing both the salvaged origin and a new-part equivalent for every component, firmware, and a filmed build. The goal is not one pen but a reproducible recipe — proof that a working additive manufacturing tool can be recovered from e-waste and plastic waste, in a region where new filament and imported hardware are prohibitively expensive.

*(Character count: ~1,520 — trim the final sentence if the limit is hard.)*

---

## 2. Documentation structure

Built to Hackster's content guidelines, which are also a good checklist for the TME entry.

### Metadata fields

| Field | Draft |
|---|---|
| **Name** | FlowForge: An Open-Source 3D Printing Pen Built Entirely From Waste |
| **Pitch** | A handheld 3D pen made from a dead 3D printer, printing filament made from plastic bottles. |
| **Cover image** | The pen mid-draw, molten bead visible, clean lighting, no text overlay. Shoot this last. |
| **Difficulty** | Intermediate |
| **Categories** | Sustainability · 3D Printing · Reuse *(max 3; no component names)* |
| **Things** | Every component listed individually with links, plus Software/Tools separately |
| **License** | CERN-OHL-S v2 (hardware) + MIT (firmware) |

Hackster explicitly wants the cover image to be high resolution, well lit, text-free, and a picture of the finished result — not a tangle of wires. Budget real time for the photography.

### Story — section outline

**1. The problem (200–300 words)**
Two waste streams that never meet: dead 3D printers whose hot ends and steppers still work, and PET bottles. Meanwhile a 3D pen costs more than most makers in Yaoundé will spend, and imported filament costs more again. Open with a concrete local number.

**2. The idea: closing the loop (150–250 words)**
The circularity argument. Diagram: bottle → strip → filament → FlowForge → printed part → (back to feedstock). This is the section that wins the contest — put the loop diagram here.

**3. What is recycled, and from what (table + 200 words)**
The provenance table. One row per component:

| Part | Salvaged from | Condition / prep | New equivalent | New cost (XAF) |
|---|---|---|---|---|
| Hot end | Donor FDM printer | Nozzle cleared, thermistor retained | E3D V6 clone | |
| Nozzle | Donor FDM printer | 0.4 mm, cold-pulled | Brass 0.4 mm | |
| Extruder mechanism | Donor FDM printer | Idler spring replaced | MK8 direct drive | |
| NEMA 17 stepper | Donor FDM printer | Bench tested | NEMA 17 42×34 | |
| PTFE tube | Donor FDM printer | Cut to length, ends faced | 2×4 mm PTFE | |
| Filament | PET bottles | Washed, dried, strip-cut | rPETG 1.75 mm | |
| Enclosures | Recycled filament | Printed in-house, 74 g | — | 7,417 |

This table does triple duty: it proves the zero-waste claim, it lets a stranger rebuild the project, and it gives you the cost-saving numbers.

**4. System architecture (250–350 words)**
The three units — pen, electronics unit, bench supply — and why the design is tethered rather than self-contained (weight, thermal isolation, and reuse of the donor printer's control stack). Block diagram here.

**5. Mechanical design (300–400 words)**
Hot end assembly cross-section with the measured stack: A_D 15.7 / A_L 3.1, B_D 11.9 / B_L 5.7, C_D 15.9 / C_L 3.7 mm. Handle ergonomics and heat isolation. Electronics unit envelope: 100 × 80 × 50/40 mm wedge. Fusion 360 renders and the exploded animation.

**6. Electronics (250–350 words)**
Arduino Mega + RAMPS 1.6 — and state plainly that this is the donor printer's own controller, reused rather than replaced. Thermistor, motor driver, fan, coupling capacitor, OLED, rotary encoder. KiCad schematic + wiring diagram. Note the Xiao ESP32-C3 miniaturisation roadmap.

**7. Firmware (200–300 words)**
PID temperature loop, thermal-runaway watchdog, encoder-driven UI, extrude control. Code as proper snippets, never plain text. Link the repo.

**8. Working with rPET filament (300–400 words) — do not skip this**
Drying protocol, temperature window, diameter tolerance you achieved and how you measured it, jam modes and fixes. This section is what separates a claim from a result, and it's the most useful thing you can give a reader.

**9. Safety (150–200 words)**
Hot end temperatures, burn risk, handle isolation, thermal runaway protection, PET fume ventilation, mains isolation at the bench supply. Short, specific, unglamorous — and it signals maturity.

**10. Build it yourself (400–600 words)**
Numbered, photographed steps. Print settings, assembly order, wiring, firmware flash, first-run calibration, first draw.

**11. Results and impact (200–300 words)**
The numbers table: grams PET diverted, e-waste mass rescued, total build cost in XAF versus a commercial pen plus virgin filament, print time, power draw.

**12. What went wrong (150–250 words)**
Honest failure log. Rare in contest entries and disproportionately persuasive.

**13. Roadmap (100–150 words)**
Untethered version, ESP32-C3, desktop companion, integrated bottle-to-filament extruder.

### Supporting uploads
- **Schematics** — KiCad exports only, no placeholders
- **CAD** — STEP + STLs + Fusion 360 source
- **Code** — PlatformIO project, correct language tags
- **Video** — YouTube build film per your existing five-scene plan

---

## Open items to resolve

1. Confirm the live contest deadline, prize split and submission format from techmasterevent.com in a browser — the site is JS-rendered and could not be read programmatically.
2. Reconcile the print mass total: the stated 74.17 g only works if the bottom frame body is counted **once** (27.54 g), not ×2. Fix before publication.
3. Decide and declare the hardware and firmware licences before first publication.
4. Measure and record actual rPET diameter tolerance — this number belongs in the abstract if it is good.
