#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include "Settings.h"

/**
 * Persistence
 *
 * Saves and restores:
 *   - Settings (language, unit system) — written on change
 *   - Service time (total minutes) — written every PERSIST_INTERVAL_MS
 *
 * Uses ESP32 NVS via the Arduino Preferences library.
 * All data survives power cycles and OTA updates.
 */
class Persistence {
public:
    void begin();

    // Settings
    void     saveSettings(const Settings& s);
    Settings loadSettings();

    // Service time
    void     saveServiceMinutes(uint32_t minutes);
    uint32_t loadServiceMinutes();

    // Lifetime pulse count (gallon metering)
    void     savePulseCount(uint32_t pulses);
    uint32_t loadPulseCount();

private:
    Preferences _prefs;
};
