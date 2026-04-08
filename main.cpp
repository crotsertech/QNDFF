// QNDFF v1.0 — Quick 'n Dirty Flowmeter Firmware
// Copyright (C) 2026 N. T. Crotser <ntc@crotser.dev>
// SPDX-License-Identifier: GPL-3.0-or-later
// Source: https://github.com/ntcrotser
//
// Hardware:
//   - GPIO 26 → KY-019 relay signal pin (active HIGH)
//   - GPIO 18 → YF-B10 flow sensor signal wire (yellow)
//   - GPIO 21 → OLED SDA
//   - GPIO 22 → OLED SCL (SCK)
//   - KY-019 VCC → 5V,  GND → GND
//   - YF-B10 VCC → 5V,  GND → GND
//   - OLED VDD  → 3V3, GND → GND
//
// WiFi AP: connect to "RelayControl" (pw: 12345678) → http://192.168.4.1

#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "Config.h"
#include "Settings.h"
#include "Persistence.h"
#include "RelayController.h"
#include "WebInterface.h"
#include "Display.h"

// ── Globals ───────────────────────────────────────────────────────────────────
RelayController relay(RELAY_PIN, FLOW_TIMEOUT_MS);
AsyncWebServer  server(80);
Display         display;
Persistence     persistence;
Settings        settings;

// Service time tracking
uint32_t serviceMinutes   = 0;
uint32_t lastMinuteMs     = 0;
uint32_t lastPersistMs    = 0;

// ── ISR ───────────────────────────────────────────────────────────────────────
void IRAM_ATTR flowPulseISR() {
    relay.onFlowPulse();
}

// ── setup ─────────────────────────────────────────────────────────────────────
void setup() {
    Serial.begin(115200);
    delay(100);   // let UART settle before first print
    Serial.printf("\n[BOOT] %s v%s\n", FW_NAME, FW_VERSION);
    Serial.printf("[BOOT] Copyright 2026 %s <%s>\n", FW_AUTHOR, FW_EMAIL);

    // Persistence — load saved settings, service time and pulse count
    persistence.begin();
    settings       = persistence.loadSettings();
    serviceMinutes = persistence.loadServiceMinutes();
    Serial.printf("[BOOT] Service time restored: %lu minutes\n", (unsigned long)serviceMinutes);

    // Relay first — safe GPIO state before anything else
    relay.begin();
    relay.setInitialPulseCount(persistence.loadPulseCount());

    // I2C — small settling delay lets OLED power rail stabilise
    Wire.begin();
    delay(50);
    display.begin();

    // Flow sensor interrupt
    pinMode(FLOW_SENSOR_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), flowPulseISR, FALLING);
    Serial.printf("[Flow] Sensor interrupt attached on GPIO%d\n", FLOW_SENSOR_PIN);

    // WiFi AP with timeout guard
    WiFi.softAP(WIFI_SSID, WIFI_PASS);
    uint32_t wifiStart = millis();
    while (WiFi.softAPIP() == IPAddress(0,0,0,0) && millis() - wifiStart < 5000) {
        delay(10);
    }
    if (WiFi.softAPIP() == IPAddress(0,0,0,0)) {
        Serial.println("[WiFi] WARNING: AP IP not assigned after 5s — rebooting");
        ESP.restart();
    }
    Serial.printf("[WiFi] AP started. SSID: %s  IP: %s\n",
                  WIFI_SSID, WiFi.softAPIP().toString().c_str());

    WebInterface::registerRoutes(server, relay, settings, persistence, serviceMinutes);
    server.begin();
    Serial.println("[HTTP] Server started on port 80");
    Serial.println("[BOOT] Ready.");

    lastMinuteMs  = millis();
    lastPersistMs = millis();
}

// ── loop ──────────────────────────────────────────────────────────────────────
void loop() {
    uint32_t now = millis();

    relay.update();
    display.update(relay, settings, serviceMinutes);

    // Increment service counter every minute
    if (now - lastMinuteMs >= 60000) {
        serviceMinutes++;
        lastMinuteMs = now;
    }

    // Persist service time and pulse count periodically
    if (now - lastPersistMs >= PERSIST_INTERVAL_MS) {
        persistence.saveServiceMinutes(serviceMinutes);
        persistence.savePulseCount(relay.getPulseCount());
        lastPersistMs = now;
    }
}
