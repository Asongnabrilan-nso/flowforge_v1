#pragma once
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "UiModel.h"
#include "../include/config.h"

// Thin wrapper around Adafruit_SSD1306. Redraws are rate-limited by UI.cpp
// (DISPLAY_REFRESH_MS) since a full-frame I2C flush takes a few ms and the
// encoder is polled in software - see Encoder.h for why that matters.
class Display {
public:
    bool begin();               // returns false if the SSD1306 didn't ACK on I2C
    void showBootScreen();       // blocking - call once from setup()
    void render(const UiModel& model);

private:
    void drawHome(const UiModel& m);
    void drawMenuMain(const UiModel& m);
    void drawMenuTemp(const UiModel& m);
    void drawMenuSettings(const UiModel& m);
    void drawEditTemp(const UiModel& m);
    void drawEditSpeed(const UiModel& m);
    void drawFault(const UiModel& m);

    void drawMenuList(const UiModel& m, const __FlashStringHelper* title,
                       const char* const labels[], const char* const values[], uint8_t count);
    void drawHeaterIcon(const UiModel& m, int16_t cx, int16_t cy);
    void drawMotorIcon(const UiModel& m, int16_t cx, int16_t cy);

    Adafruit_SSD1306 _oled{OLED_WIDTH, OLED_HEIGHT, &Wire, -1};
};

extern Display display;
