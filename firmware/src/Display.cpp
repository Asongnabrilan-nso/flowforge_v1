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

void Display::render(const UiModel& m) {
    _oled.clearDisplay();
    _oled.setTextColor(SSD1306_WHITE);

    if (m.heaterState == HeaterState::FAULT) {
        drawFault(m);
    } else {
        switch (m.screen) {
            case Screen::HOME:  drawHome(m);  break;
            case Screen::TEMP:  drawTemp(m);  break;
            case Screen::SPEED: drawSpeed(m); break;
        }
    }

    _oled.display();
}

void Display::drawHeaderIcon(const UiModel& m) {
    _oled.setTextSize(1);
    _oled.setCursor(0, 0);
    _oled.print(heaterStateLabel(m.heaterState));

    const char* extLabel = m.extruding ? "EXTRUDING" : "";
    int16_t w = strlen(extLabel) * 6;
    _oled.setCursor(OLED_WIDTH - w, 0);
    _oled.print(extLabel);
}

void Display::drawHome(const UiModel& m) {
    drawHeaderIcon(m);

    _oled.setTextSize(2);
    _oled.setCursor(0, 14);
    _oled.print((int)round(m.currentTempC));
    _oled.print((char)247); // degree symbol (cp437)
    _oled.print('C');

    _oled.setTextSize(1);
    _oled.setCursor(0, 34);
    _oled.print(F("Target: "));
    _oled.print((int)round(m.targetTempC));
    _oled.print((char)247);
    _oled.print('C');

    _oled.setCursor(0, 46);
    _oled.print(F("Speed: "));
    _oled.print(m.speedLevel);
    _oled.print(F("/"));
    _oled.print(SPEED_LEVEL_MAX);

    if (!m.safeToExtrude) {
        _oled.setCursor(0, 56);
        _oled.print(F("Too cold to extrude"));
    }
}

void Display::drawTemp(const UiModel& m) {
    _oled.setTextSize(1);
    _oled.setCursor(0, 0);
    _oled.print(F("SET TEMPERATURE"));

    char buf[8];
    snprintf(buf, sizeof(buf), "%dC", (int)round(m.targetTempC));
    _oled.setTextSize(3);
    int16_t textW = strlen(buf) * 18; // size-3 char cell is 18px wide
    int16_t x = (OLED_WIDTH - textW) / 2;
    if (m.editing) {
        _oled.drawRect(x - 4, 18, textW + 8, 30, SSD1306_WHITE);
    }
    _oled.setCursor(x, 22);
    _oled.print(buf);

    _oled.setTextSize(1);
    _oled.setCursor(0, 54);
    _oled.print(F("now "));
    _oled.print((int)round(m.currentTempC));
    _oled.print((char)247);
    _oled.print('C');
}

void Display::drawSpeed(const UiModel& m) {
    _oled.setTextSize(1);
    _oled.setCursor(0, 0);
    _oled.print(F("SET SPEED"));

    _oled.setTextSize(3);
    char buf[4];
    snprintf(buf, sizeof(buf), "%d", m.speedLevel);
    int16_t textW = strlen(buf) * 18;
    int16_t x = (OLED_WIDTH - textW) / 2;
    if (m.editing) {
        _oled.drawRect(x - 4, 14, textW + 8, 30, SSD1306_WHITE);
    }
    _oled.setCursor(x, 18);
    _oled.print(buf);

    // Segmented level bar across the bottom.
    const int16_t barY = 52, segW = 10, gap = 2;
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
