#pragma once

// ── QNDFF — Quick 'n Dirty Flowmeter Firmware ────────────────────────────────
// Copyright (C) 2026 N. T. Crotser <ntc@crotser.dev>
// SPDX-License-Identifier: GPL-3.0-or-later
// Source: https://github.com/ntcrotser
//
// This program is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License v3 as published by the
// Free Software Foundation. See <https://www.gnu.org/licenses/>.

constexpr const char* FW_NAME      = "QNDFF";
constexpr const char* FW_VERSION   = "1.0";
constexpr const char* FW_AUTHOR    = "N. T. Crotser";
constexpr const char* FW_EMAIL     = "ntc@crotser.dev";
//constexpr const char* FW_GITHUB    = "https://github.com/ntcrotser";
//constexpr const char* FW_LICENSE   = "GPL 3.0+";
//constexpr const char* FW_FULL_NAME = "Quick 'n Dirty Flowmeter Firmware v1.0 by N. T. Crotser <";

// ── WiFi Access Point ─────────────────────────────────────────────────────────
constexpr const char* WIFI_SSID = "RelayControl";
constexpr const char* WIFI_PASS = "12345678";   // min 8 chars for WPA2

// ── Relay Hardware ────────────────────────────────────────────────────────────
constexpr uint8_t RELAY_PIN = 26;               // GPIO driving KY-019 signal

// ── OLED Display (SSD1306 I2C) ────────────────────────────────────────────────
constexpr uint8_t  OLED_I2C_ADDR    = 0x3C;    // try 0x3D if display is blank
constexpr uint8_t  OLED_WIDTH       = 128;
constexpr uint8_t  OLED_HEIGHT      = 64;
constexpr uint32_t DISPLAY_UPDATE_MS = 250;     // refresh interval

// ── Flow Sensor (YF-B10) ──────────────────────────────────────────────────────
// Datasheet pulse characteristic: F = (6*Q - 8), F=Hz, Q=L/min
// At typical flow rates this works out to ~5.5 pulses/litre.
// Calibrate by running a known volume through the sensor and adjusting this value.
constexpr uint8_t  FLOW_SENSOR_PIN       = 18;    // GPIO for YF-B10 signal wire
constexpr uint32_t FLOW_TIMEOUT_MS       = 300;   // ms without a pulse → no flow
constexpr float    FLOW_PULSES_PER_LITRE = 5.5f;  // adjust after calibration
constexpr float    LITRES_PER_GALLON     = 3.78541f;

// ── Persistence ───────────────────────────────────────────────────────────────
constexpr uint32_t PERSIST_INTERVAL_MS = 60000; // save service time every 60s
