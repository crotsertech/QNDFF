// QNDFF v1.0 — Quick 'n Dirty Flowmeter Firmware
// Copyright (C) 2026 N. T. Crotser <ntc@crotser.dev>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Display.h"
#include "Config.h"
#include <Adafruit_GFX.h>

// ── λ bitmap (16x16, Half-Life lambda) ───────────────────────────────────────
static const uint8_t LAMBDA_BMP[] PROGMEM = {
    0b00000001, 0b10000000,
    0b00000001, 0b10000000,
    0b00000011, 0b00000000,
    0b00000111, 0b00000000,
    0b00000110, 0b00000000,
    0b00001110, 0b00000000,
    0b00001100, 0b00000000,
    0b00011100, 0b00000000,
    0b00111110, 0b00000000,
    0b00110011, 0b00000000,
    0b01100001, 0b10000000,
    0b01000000, 0b11000000,
    0b11000000, 0b01100000,
    0b10000000, 0b00110000,
    0b00000000, 0b00011000,
    0b00000000, 0b00001100,
};
static constexpr uint8_t LAMBDA_W = 16;
static constexpr uint8_t LAMBDA_H = 16;

// ── © bitmap (8x8) ────────────────────────────────────────────────────────────
static const uint8_t COPYRIGHT_BMP[] PROGMEM = {
    0b00111100,  // row 0:  .XXX XX.
    0b01000010,  // row 1: X.....X.
    0b10011101,  // row 2: X..XXX.X  (circle with C inside)
    0b10100001,  // row 3: X.X....X
    0b10100001,  // row 4: X.X....X
    0b10011101,  // row 5: X..XXX.X
    0b01000010,  // row 6: X.....X.
    0b00111100,  // row 7:  .XXXX .
};
static constexpr uint8_t COPYRIGHT_W = 8;
static constexpr uint8_t COPYRIGHT_H = 8;

// ── Flow pattern ──────────────────────────────────────────────────────────────
static constexpr uint8_t  FLOW_COLS       = 11;
static constexpr uint8_t  FLOW_PERIOD     = 2;
static constexpr uint8_t  SCROLL_SLOWDOWN = 2;

// ── Localised strings ─────────────────────────────────────────────────────────
// Ukrainian
static const char UK_CLOSED[]      = "Реле : ЗАКРИТО";
static const char UK_OPEN[]        = "Реле : ВІДКРИТО";
static const char UK_FLOWSWITCH[]  = "Режим: Витратомір";   // Flowmeter
static const char UK_OVERRIDE[]    = "Режим: РУЧНИЙ";
static const char UK_GAL[]         = " Гал Виміряно";
static const char UK_LIT[]         = " Л Виміряно";
static const char UK_SVC[]         = "Жит:";

// Chinese (Simplified)
static const char ZH_CLOSED[]      = "继电器: 闭合";
static const char ZH_OPEN[]        = "继电器: 断开";
static const char ZH_FLOWSWITCH[]  = "模式: 流量计";     // Flowmeter
static const char ZH_OVERRIDE[]    = "模式: 手动";
static const char ZH_GAL[]         = " 加仑 已计量";
static const char ZH_LIT[]         = " 升 已计量";
static const char ZH_SVC[]         = "寿命:";

// ── Constructor ───────────────────────────────────────────────────────────────
Display::Display()
    : _oled(OLED_WIDTH, OLED_HEIGHT, &Wire, -1) {}

bool Display::begin() {
    // Retry I2C init up to 5 times — OLED may not be ready immediately after power-on
    uint8_t attempts = 0;
    while (!_oled.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR)) {
        attempts++;
        if (attempts >= 5) {
            Serial.printf("[OLED] Not found at 0x%02X after %d attempts — check wiring or try 0x3D\n",
                          OLED_I2C_ADDR, attempts);
            return false;
        }
        Serial.printf("[OLED] Init attempt %d failed, retrying...\n", attempts);
        delay(100);
    }
    _oled.setTextWrap(false);
    _oled.clearDisplay();
    _oled.setTextColor(SSD1306_WHITE);

    _u8g2.begin(_oled);
    _u8g2.setFontMode(1);
    _u8g2.setFontDirection(0);
    _u8g2.setForegroundColor(SSD1306_WHITE);
    _u8g2.setBackgroundColor(SSD1306_BLACK);
    _u8g2.setFont(u8g2_font_6x13_t_cyrillic);

    // ── Splash phase 1: branding (2500ms) ────────────────────────────────────
    _oled.setTextSize(1);
    _oled.setCursor(4, 10);
    _oled.print(FW_NAME);
    _oled.print(" v.");
    _oled.print(FW_VERSION);
    _oled.setCursor(4, 28);
    _oled.drawBitmap(4, 28, COPYRIGHT_BMP, COPYRIGHT_W, COPYRIGHT_H, SSD1306_WHITE);
    _oled.setCursor(16, 28);
    _oled.print(" 2026");
    _oled.setCursor(4, 40);
    _oled.print(FW_AUTHOR);
    _oled.display();

    uint32_t t = millis();
    while (millis() - t < 2500) { yield(); }
    _oled.clearDisplay();

    // ── Splash phase 2: motivational flash (~120ms — almost too fast to read) ─
    _oled.setTextSize(1);
    _oled.setCursor(4, 10);
    _oled.print("KEEP GOING,");
    _oled.setCursor(4, 26);
    _oled.print("THE HORIZON");
    _oled.setCursor(4, 42);
    _oled.print("IS WAITING!");
    _oled.display();

    t = millis();
    while (millis() - t < 120) { yield(); }
    _oled.clearDisplay();

    _ready = true;
    Serial.printf("[OLED] Initialized at 0x%02X\n", OLED_I2C_ADDR);
    return true;
}

void Display::update(const RelayController& relay,
                     const Settings& settings,
                     uint32_t serviceMinutes) {
    if (!_ready) return;

    uint32_t now = millis();
    if (now - _lastUpdateMs < DISPLAY_UPDATE_MS) return;
    _lastUpdateMs = now;

    _scrollTick = (_scrollTick + 1) % SCROLL_SLOWDOWN;
    if (_scrollTick == 0) {
        _scrollOffset = (_scrollOffset + 1) % FLOW_PERIOD;
    }

    render(relay, settings, serviceMinutes);
}

// ── Helpers ───────────────────────────────────────────────────────────────────

String Display::flowString() {
    static const char TILE[] = ">-";
    String s;
    s.reserve(FLOW_COLS);
    for (uint8_t i = 0; i < FLOW_COLS; i++) {
        s += TILE[(_scrollOffset + i) % FLOW_PERIOD];
    }
    return s;
}

String Display::formatVolume(uint32_t pulses, UnitSystem units) {
    float litres = pulses / FLOW_PULSES_PER_LITRE;
    char buf[24];
    if (units == UnitSystem::Metric) {
        snprintf(buf, sizeof(buf), "%.2f L", litres);
    } else {
        snprintf(buf, sizeof(buf), "%.2f Gal", litres / LITRES_PER_GALLON);
    }
    return String(buf);
}

String Display::formatUptime(uint32_t minutes) {
    char buf[32];
    if (minutes < 60) {
        snprintf(buf, sizeof(buf), "%lum", (unsigned long)minutes);
    } else if (minutes < 1440) {
        snprintf(buf, sizeof(buf), "%luh %lum",
                 (unsigned long)(minutes / 60),
                 (unsigned long)(minutes % 60));
    } else if (minutes < 43200) {
        uint32_t days  = minutes / 1440;
        uint32_t hours = (minutes % 1440) / 60;
        snprintf(buf, sizeof(buf), "%lud %luh", (unsigned long)days, (unsigned long)hours);
    } else if (minutes < 525600) {
        uint32_t months = minutes / 43200;
        uint32_t days   = (minutes % 43200) / 1440;
        snprintf(buf, sizeof(buf), "%lumo %lud", (unsigned long)months, (unsigned long)days);
    } else {
        uint32_t years  = minutes / 525600;
        uint32_t months = (minutes % 525600) / 43200;
        snprintf(buf, sizeof(buf), "%luyr %lumo", (unsigned long)years, (unsigned long)months);
    }
    return String(buf);
}

// Print UTF-8 string via U8g2 at top-left pixel coordinate
void Display::printU8(int16_t x, int16_t y, const char* utf8str) {
    _u8g2.setCursor(x, y + 11);  // +11 = ascender of 6x13 font
    _u8g2.print(utf8str);
}

// ── Render ────────────────────────────────────────────────────────────────────
void Display::render(const RelayController& relay,
                     const Settings& settings,
                     uint32_t serviceMinutes) {
    bool relayOn  = (relay.getRelayState() == RelayController::State::ON);
    bool override = (relay.getMode()       == RelayController::Mode::OVERRIDE);

    String volume  = formatVolume(relay.getPulseCount(), settings.units);
    String uptime  = formatUptime(serviceMinutes);

    _oled.clearDisplay();

    // ── Row 0: always ASCII animated ─────────────────────────────────────────
    _oled.setFont(nullptr);
    _oled.setTextSize(2);
    _oled.setCursor(0, 0);
    _oled.print(relayOn ? flowString() : "-----------");

    // ── Rows 1-4: localised content ───────────────────────────────────────────
    switch (settings.language) {

        case Language::English: {
            _oled.setTextSize(1);
            _oled.setCursor(0, 20);
            _oled.print(relayOn ? "Relay : CLOSED" : "Relay : OPEN");
            _oled.setCursor(0, 32);
            _oled.print(override ? "Mode  : OVERRIDE" : "Mode  : Flowmeter");
            _oled.setCursor(0, 44);
            _oled.print(volume);
            _oled.print(" Metered");
            _oled.setCursor(0, 56);
            _oled.print("Lifetime: ");
            _oled.print(uptime);
            break;
        }

        case Language::Ukrainian: {
            printU8(0, 20, relayOn ? UK_CLOSED : UK_OPEN);
            printU8(0, 32, override ? UK_OVERRIDE : UK_FLOWSWITCH);
            {
                char vbuf[32];
                float litres = relay.getPulseCount() / FLOW_PULSES_PER_LITRE;
                if (settings.units == UnitSystem::Metric) {
                    snprintf(vbuf, sizeof(vbuf), "%.2f%s", litres, UK_LIT);
                } else {
                    snprintf(vbuf, sizeof(vbuf), "%.2f%s", litres / LITRES_PER_GALLON, UK_GAL);
                }
                printU8(0, 44, vbuf);
            }
            // Service line: print short Cyrillic prefix then switch to small ASCII font for value
            printU8(0, 56, UK_SVC);
            _oled.setFont(nullptr);
            _oled.setTextSize(1);
            _oled.setCursor(26, 56);   // "Жит:" ≈ 4 chars × 6px = 24px
            _oled.print(uptime);
            break;
        }

        case Language::Chinese: {
            _u8g2.setFont(u8g2_font_wqy12_t_gb2312);
            printU8(0, 20, relayOn ? ZH_CLOSED : ZH_OPEN);
            printU8(0, 32, override ? ZH_OVERRIDE : ZH_FLOWSWITCH);
            {
                char vbuf[32];
                float litres = relay.getPulseCount() / FLOW_PULSES_PER_LITRE;
                if (settings.units == UnitSystem::Metric) {
                    snprintf(vbuf, sizeof(vbuf), "%.2f%s", litres, ZH_LIT);
                } else {
                    snprintf(vbuf, sizeof(vbuf), "%.2f%s", litres / LITRES_PER_GALLON, ZH_GAL);
                }
                printU8(0, 44, vbuf);
            }
            // Service line: short CJK prefix (服务:) then ASCII value
            printU8(0, 56, ZH_SVC);
            _oled.setFont(nullptr);
            _oled.setTextSize(1);
            _oled.setCursor(26, 56);   // "服务:" ≈ 2 CJK + colon = ~26px
            _oled.print(uptime);
            _u8g2.setFont(u8g2_font_6x13_t_cyrillic);
            break;
        }
    }

    // ── Lambda: top-right corner in override mode ─────────────────────────────
    if (override) {
        _oled.drawBitmap(
            OLED_WIDTH - LAMBDA_W - 1,
            1,
            LAMBDA_BMP, LAMBDA_W, LAMBDA_H,
            SSD1306_WHITE
        );
    }

    _oled.display();
}
