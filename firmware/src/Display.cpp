#include "Display.h"
#include <string.h>
#include <stdio.h>
#include "../include/flowforge_logo.h"

Display display;

bool Display::begin() {
    Wire.begin();
    if (!_oled.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR)) {
        return false;
    }
    _oled.setTextColor(SSD1306_WHITE);
    _oled.cp437(true);
    return true;
}

void Display::showBootScreen() {
    _oled.clearDisplay();
    int16_t x = (OLED_WIDTH - FLOWFORGE_LOGO_WIDTH) / 2;
    _oled.drawBitmap(x, 4, flowforge_logo_bmp, FLOWFORGE_LOGO_WIDTH, FLOWFORGE_LOGO_HEIGHT, SSD1306_WHITE);
    _oled.setTextSize(1);
    _oled.setCursor(0, 40);
    //_oled.print(F("Recycled 3D printing pen"));

    // Simple non-animated progress bar so the boot pause reads as "loading"
    // rather than a frozen screen. Blocking is fine here - nothing else runs yet.
    const int16_t barX = 14, barY = 54, barW = 100, barH = 6;
    _oled.drawRect(barX, barY, barW, barH, SSD1306_WHITE);
    _oled.display();
    for (int16_t w = 0; w <= barW - 2; w += 4) {
        _oled.fillRect(barX + 1, barY + 1, w, barH - 2, SSD1306_WHITE);
        _oled.display();
        delay(20);
    }
    delay(300);
}

static const __FlashStringHelper* heaterStateLabel(HeaterState s) {
    switch (s) {
        case HeaterState::IDLE:    return F("IDLE");
        case HeaterState::HEATING: return F("HEATING");
        case HeaterState::AT_TEMP: return F("READY");
        case HeaterState::FAULT:   return F("FAULT!");
    }
    return F("");
}

static const __FlashStringHelper* faultReasonLabel(FaultReason r) {
    switch (r) {
        case FaultReason::SENSOR:           return F("Thermistor fault");
        case FaultReason::OVER_TEMP:        return F("Over-temperature");
        case FaultReason::RUNAWAY_NO_RISE:  return F("Not heating - check wiring");
        case FaultReason::RUNAWAY_DRIFT:    return F("Temp drift - heater stuck?");
        default:                            return F("Unknown fault");
    }
}

// How far current temp has progressed toward target, 0-100. Hidden by
// callers whenever target is at/below TEMP_SETPOINT_MIN_C (heater idle).
static uint8_t heatProgressPercent(const UiModel& m) {
    if (m.targetTempC <= TEMP_SETPOINT_MIN_C) return 0;
    float pct = (m.currentTempC / m.targetTempC) * 100.0f;
    if (pct < 0.0f) pct = 0.0f;
    if (pct > 100.0f) pct = 100.0f;
    return (uint8_t)pct;
}

static void drawProgressBar(Adafruit_SSD1306& oled, int16_t x, int16_t y,
                             int16_t w, int16_t h, uint8_t percent) {
    oled.drawRect(x, y, w, h, SSD1306_WHITE);
    int16_t fillW = (int16_t)((int32_t)(w - 2) * percent / 100);
    if (fillW > 0) oled.fillRect(x + 1, y + 1, fillW, h - 2, SSD1306_WHITE);
}

void Display::render(const UiModel& m) {
    _oled.clearDisplay();
    _oled.setTextColor(SSD1306_WHITE);

    if (m.heaterState == HeaterState::FAULT) {
        drawFault(m);
    } else {
        switch (m.screen) {
            case Screen::HOME:          drawHome(m);          break;
            case Screen::MENU_MAIN:     drawMenuMain(m);      break;
            case Screen::MENU_TEMP:     drawMenuTemp(m);      break;
            case Screen::MENU_SETTINGS: drawMenuSettings(m);  break;
            case Screen::EDIT_TEMP:     drawEditTemp(m);      break;
            case Screen::EDIT_SPEED:    drawEditSpeed(m);     break;
        }
    }

    _oled.display();
}

// Hollow = idle, blinking = heating, filled = at temperature. FAULT never
// reaches here - render() dispatches to drawFault() first.
void Display::drawHeaterIcon(const UiModel& m, int16_t cx, int16_t cy) {
    const int16_t r = 6;
    switch (m.heaterState) {
        case HeaterState::AT_TEMP:
            _oled.fillCircle(cx, cy, r, SSD1306_WHITE);
            break;
        case HeaterState::HEATING:
            if ((millis() / 500) % 2 == 0) _oled.fillCircle(cx, cy, r, SSD1306_WHITE);
            else _oled.drawCircle(cx, cy, r, SSD1306_WHITE);
            break;
        default: // IDLE
            _oled.drawCircle(cx, cy, r, SSD1306_WHITE);
            break;
    }
}

// Simple hand-drawn "gear" (ring + 4 spokes) - center dot fills while
// actively extruding, mirrors drawHeaterIcon's on/off read-at-a-glance style.
void Display::drawMotorIcon(const UiModel& m, int16_t cx, int16_t cy) {
    const int16_t r = 6;
    _oled.drawCircle(cx, cy, r, SSD1306_WHITE);
    _oled.drawLine(cx, cy - r, cx, cy - r - 3, SSD1306_WHITE);
    _oled.drawLine(cx, cy + r, cx, cy + r + 3, SSD1306_WHITE);
    _oled.drawLine(cx - r, cy, cx - r - 3, cy, SSD1306_WHITE);
    _oled.drawLine(cx + r, cy, cx + r + 3, cy, SSD1306_WHITE);
    if (m.extruding) _oled.fillCircle(cx, cy, 2, SSD1306_WHITE);
}

void Display::drawHome(const UiModel& m) {
    // Left: heater icon + target-over-actual temps.
    drawHeaterIcon(m, 9, 9);
    _oled.setTextSize(1);
    _oled.setCursor(18, 0);
    _oled.print((int)round(m.targetTempC));
    _oled.print((char)247); // degree symbol (cp437)
    _oled.print('C');
    _oled.setCursor(18, 9);
    _oled.print((int)round(m.currentTempC));
    _oled.print((char)247);
    _oled.print('C');

    // Right: motor icon + target-level-over-actual-feedrate.
    drawMotorIcon(m, 118, 9);
    char speedBuf[10];
    snprintf(speedBuf, sizeof(speedBuf), "L%d/%d", (int)m.speedLevel, (int)SPEED_LEVEL_MAX);
    _oled.setCursor(62, 0);
    _oled.print(speedBuf);
    char feedBuf[10];
    int feedTenths = (int)(m.speedCurrentMmS * 10.0f) % 10;
    snprintf(feedBuf, sizeof(feedBuf), "%d.%01dmm/s", (int)m.speedCurrentMmS, feedTenths);
    _oled.setCursor(62, 9);
    _oled.print(feedBuf);

    // Status row.
    _oled.setCursor(0, 21);
    _oled.print(heaterStateLabel(m.heaterState));
    if (m.extruding) _oled.print(F(" EXTRUDING"));

    // Heat-progress bar + extrude-session elapsed time.
    if (m.targetTempC > TEMP_SETPOINT_MIN_C) {
        drawProgressBar(_oled, 4, 31, 74, 8, heatProgressPercent(m));
    }
    char timeBuf[8];
    snprintf(timeBuf, sizeof(timeBuf), "%02u:%02u", m.extrudeElapsedS / 60, m.extrudeElapsedS % 60);
    _oled.setCursor(82, 32);
    _oled.print(timeBuf);

    // Nav hint - Home no longer cycles directly to Temp/Speed, so teach the
    // new indirect menu access.
    _oled.setCursor(0, 43);
    _oled.print(F("Turn:Menu Click:Run"));

    if (!m.safeToExtrude) {
        _oled.setCursor(0, 55);
        _oled.print(F("Too cold to extrude"));
    }
}

// Shared list renderer for MENU_MAIN/MENU_TEMP/MENU_SETTINGS: title line,
// up to 5 visible rows (10px each) with a scrolling window and an inverted
// highlight bar on the selected row. values[i] may be nullptr for a
// label-only row (Back, one-shot actions).
void Display::drawMenuList(const UiModel& m, const __FlashStringHelper* title,
                            const char* const labels[], const char* const values[], uint8_t count) {
    _oled.setTextSize(1);
    _oled.setTextColor(SSD1306_WHITE);
    _oled.setCursor(0, 0);
    _oled.print(title);

    const uint8_t visibleRows = 5;
    uint8_t scrollOffset = 0;
    if (m.menuSelectedIndex >= visibleRows) scrollOffset = m.menuSelectedIndex - visibleRows + 1;

    for (uint8_t i = 0; i < visibleRows && (uint8_t)(scrollOffset + i) < count; i++) {
        uint8_t idx = scrollOffset + i;
        int16_t y = 12 + i * 10;
        bool hi = (idx == m.menuSelectedIndex);
        if (hi) {
            _oled.fillRect(0, y - 1, OLED_WIDTH, 9, SSD1306_WHITE);
            _oled.setTextColor(SSD1306_BLACK);
        } else {
            _oled.setTextColor(SSD1306_WHITE);
        }
        _oled.setCursor(2, y);
        _oled.print(labels[idx]);
        if (values[idx] != nullptr) {
            int16_t vw = (int16_t)strlen(values[idx]) * 6;
            _oled.setCursor(OLED_WIDTH - vw - 2, y);
            _oled.print(values[idx]);
        }
    }
    _oled.setTextColor(SSD1306_WHITE);
}

void Display::drawMenuMain(const UiModel& m) {
    static const char* labels[] = {"Back", "Temperature", "Motor Speed", "Settings"};
    char speedVal[8];
    snprintf(speedVal, sizeof(speedVal), "%d/%d", (int)m.speedLevel, (int)SPEED_LEVEL_MAX);
    const char* values[] = {nullptr, nullptr, speedVal, nullptr};
    drawMenuList(m, F("MAIN MENU"), labels, values, 4);
}

void Display::drawMenuTemp(const UiModel& m) {
    static const char* labels[] = {"Back", "Nozzle", "Preheat PET", "Preheat HDPE", "Cooldown"};
    char nozzleVal[8], petVal[8], hdpeVal[8];
    snprintf(nozzleVal, sizeof(nozzleVal), "%dC", (int)round(m.targetTempC));
    snprintf(petVal, sizeof(petVal), "%dC", (int)PREHEAT_PET_C);
    snprintf(hdpeVal, sizeof(hdpeVal), "%dC", (int)PREHEAT_HDPE_C);
    const char* values[] = {nullptr, nozzleVal, petVal, hdpeVal, nullptr};
    drawMenuList(m, F("TEMPERATURE"), labels, values, 5);
}

void Display::drawMenuSettings(const UiModel& m) {
    static const char* labels[] = {"Back", "Steps/mm", "Max speed", "Accel"};
    char stepsVal[8], maxSpeedVal[10], accelVal[8];
    snprintf(stepsVal, sizeof(stepsVal), "%d", (int)EXTRUDER_STEPS_PER_MM);
    snprintf(maxSpeedVal, sizeof(maxSpeedVal), "%d.%01d",
             (int)EXTRUDER_FEEDRATE_MAX_MM_S, ((int)(EXTRUDER_FEEDRATE_MAX_MM_S * 10)) % 10);
    snprintf(accelVal, sizeof(accelVal), "%d", (int)EXTRUDER_ACCEL_MM_S2);
    const char* values[] = {nullptr, stepsVal, maxSpeedVal, accelVal};
    drawMenuList(m, F("SETTINGS"), labels, values, 4);
}

void Display::drawEditTemp(const UiModel& m) {
    _oled.setTextSize(1);
    _oled.setCursor(0, 0);
    _oled.print(F("SET TEMPERATURE"));

    char buf[8];
    snprintf(buf, sizeof(buf), "%dC", (int)round(m.targetTempC));
    _oled.setTextSize(3);
    int16_t textW = (int16_t)strlen(buf) * 18; // size-3 char cell is 18px wide
    int16_t x = (OLED_WIDTH - textW) / 2;
    _oled.drawRect(x - 4, 18, textW + 8, 30, SSD1306_WHITE);
    _oled.setCursor(x, 22);
    _oled.print(buf);

    if (m.targetTempC > TEMP_SETPOINT_MIN_C) {
        drawProgressBar(_oled, 4, 49, 120, 6, heatProgressPercent(m));
    }

    _oled.setTextSize(1);
    _oled.setCursor(0, 56);
    _oled.print(F("now "));
    _oled.print((int)round(m.currentTempC));
    _oled.print((char)247);
    _oled.print('C');
}

void Display::drawEditSpeed(const UiModel& m) {
    _oled.setTextSize(1);
    _oled.setCursor(0, 0);
    _oled.print(F("SET MOTOR SPEED"));

    _oled.setTextSize(3);
    char buf[4];
    snprintf(buf, sizeof(buf), "%d", (int)m.speedLevel);
    int16_t textW = (int16_t)strlen(buf) * 18;
    int16_t x = (OLED_WIDTH - textW) / 2;
    _oled.drawRect(x - 4, 14, textW + 8, 30, SSD1306_WHITE);
    _oled.setCursor(x, 18);
    _oled.print(buf);

    _oled.setTextSize(1);
    char feedBuf[10];
    int feedTenths = (int)(m.speedTargetMmS * 10.0f) % 10;
    snprintf(feedBuf, sizeof(feedBuf), "%d.%01dmm/s", (int)m.speedTargetMmS, feedTenths);
    _oled.setCursor(0, 45);
    _oled.print(feedBuf);

    // Segmented level bar across the bottom.
    const int16_t barY = 55, segW = 10, gap = 2;
    int16_t totalW = SPEED_LEVEL_MAX * (segW + gap) - gap;
    int16_t startX = (OLED_WIDTH - totalW) / 2;
    for (uint8_t i = 0; i < SPEED_LEVEL_MAX; i++) {
        int16_t sx = startX + i * (segW + gap);
        if (i < m.speedLevel) {
            _oled.fillRect(sx, barY, segW, 8, SSD1306_WHITE);
        } else {
            _oled.drawRect(sx, barY, segW, 8, SSD1306_WHITE);
        }
    }
}

void Display::drawFault(const UiModel& m) {
    _oled.setTextSize(2);
    _oled.setCursor(28, 4);
    _oled.print(F("FAULT!"));

    _oled.setTextSize(1);
    _oled.setCursor(0, 28);
    _oled.print(faultReasonLabel(m.faultReason));

    _oled.setCursor(0, 42);
    _oled.print(F("Heater & motor locked"));
    _oled.setCursor(0, 54);
    _oled.print(F("Power cycle to reset"));
}
