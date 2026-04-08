#pragma once
#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <U8g2_for_Adafruit_GFX.h>
#include "RelayController.h"
#include "Settings.h"

/**
 * Display — QNDFF v1.0
 *
 * Drives the 128x64 SSD1306 OLED over I2C.
 * Call update() from loop() — self-throttles to DISPLAY_UPDATE_MS.
 *
 * Row 0: always animated (>->- scrolling or ---- static)
 * Rows 1-3: status info in selected language (English/Ukrainian/Chinese)
 * Row 4: service uptime (mins/hours/days/months/years)
 *
 * Units switchable between US (gallons) and Metric (litres).
 */
class Display {
public:
    Display();

    bool begin();

    void update(const RelayController& relay,
                const Settings& settings,
                uint32_t serviceMinutes);

private:
    void   render(const RelayController& relay,
                  const Settings& settings,
                  uint32_t serviceMinutes);
    void   printU8(int16_t x, int16_t y, const char* utf8str);
    String flowString();
    String formatVolume(uint32_t pulses, UnitSystem units);
    String formatUptime(uint32_t minutes);

    Adafruit_SSD1306       _oled;
    U8G2_FOR_ADAFRUIT_GFX _u8g2;

    uint32_t _lastUpdateMs   = 0;
    uint8_t  _scrollOffset   = 0;
    uint8_t  _scrollTick     = 0;
    bool     _ready          = false;
};
