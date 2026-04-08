#pragma once
#include <Arduino.h>

// ── Language selection ────────────────────────────────────────────────────────
enum class Language : uint8_t {
    English  = 0,
    Ukrainian = 1,
    Chinese  = 2
};

// ── Unit system ───────────────────────────────────────────────────────────────
enum class UnitSystem : uint8_t {
    US     = 0,   // gallons
    Metric = 1    // litres
};

// ── Runtime settings (persisted to NVS) ──────────────────────────────────────
struct Settings {
    Language   language   = Language::English;
    UnitSystem units      = UnitSystem::US;
};
