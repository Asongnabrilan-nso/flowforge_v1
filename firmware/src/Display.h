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
    void drawTemp(const UiModel& m);
    void drawSpeed(const UiModel& m);
    void drawFault(const UiModel& m);
    void drawHeaderIcon(const UiModel& m);

    Adafruit_SSD1306 _oled{OLED_WIDTH, OLED_HEIGHT, &Wire, -1};
};

extern Display display;
