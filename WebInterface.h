#pragma once
#include <ESPAsyncWebServer.h>
#include "RelayController.h"
#include "Settings.h"
#include "Persistence.h"

/**
 * WebInterface — QNDFF v1.0
 *
 * Endpoints:
 *   GET  /                          → Full control page
 *   GET  /api/status                → JSON status + settings + uptime
 *   POST /api/override?enable=1|0   → Enter/exit override mode
 *   POST /api/relay?state=on|off    → Set relay (override only)
 *   POST /api/units?system=us|metric → Switch unit system
 *   POST /api/language?lang=en|uk|zh → Switch display language
 */
namespace WebInterface {
    void registerRoutes(AsyncWebServer& server,
                        RelayController& relay,
                        Settings& settings,
                        Persistence& persistence,
                        uint32_t& serviceMinutes);
}
