#include "Persistence.h"
#include <Arduino.h>

static const char* NVS_NS       = "qndff";
static const char* KEY_LANG     = "lang";
static const char* KEY_UNITS    = "units";
static const char* KEY_SVC_MIN  = "svcmin";
static const char* KEY_PULSES   = "pulses";

void Persistence::begin() {
    _prefs.begin(NVS_NS, false);
    Serial.println("[NVS] Persistence initialized");
}

void Persistence::saveSettings(const Settings& s) {
    _prefs.putUChar(KEY_LANG,  static_cast<uint8_t>(s.language));
    _prefs.putUChar(KEY_UNITS, static_cast<uint8_t>(s.units));
    Serial.printf("[NVS] Settings saved (lang=%d units=%d)\n",
                  (int)s.language, (int)s.units);
}

Settings Persistence::loadSettings() {
    Settings s;
    s.language = static_cast<Language>(_prefs.getUChar(KEY_LANG,  0));
    s.units    = static_cast<UnitSystem>(_prefs.getUChar(KEY_UNITS, 0));
    Serial.printf("[NVS] Settings loaded (lang=%d units=%d)\n",
                  (int)s.language, (int)s.units);
    return s;
}

void Persistence::saveServiceMinutes(uint32_t minutes) {
    _prefs.putULong(KEY_SVC_MIN, minutes);
}

uint32_t Persistence::loadServiceMinutes() {
    return _prefs.getULong(KEY_SVC_MIN, 0);
}

void Persistence::savePulseCount(uint32_t pulses) {
    _prefs.putULong(KEY_PULSES, pulses);
}

uint32_t Persistence::loadPulseCount() {
    return _prefs.getULong(KEY_PULSES, 0);
}
