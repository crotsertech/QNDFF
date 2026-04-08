// QNDFF v1.0 — Quick 'n Dirty Flowmeter Firmware
// Copyright (C) 2026 N. T. Crotser <ntc@crotser.dev>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "WebInterface.h"
#include "Config.h"
#include <Arduino.h>

// ── HTML page ─────────────────────────────────────────────────────────────────
static const char INDEX_HTML[] PROGMEM = R"rawhtml(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8"/>
  <meta name="viewport" content="width=device-width,initial-scale=1"/>
  <title>QNDFF v1.0</title>
  <style>
    *{box-sizing:border-box}
    body{font-family:sans-serif;max-width:420px;margin:40px auto;padding:0 16px;background:#f0f2f5}
    h1{font-size:1.3rem;margin-bottom:2px;color:#1a1a2e}
    .sub{font-size:.75rem;color:#888;margin-bottom:12px}
    .card{background:#fff;border-radius:12px;padding:18px;box-shadow:0 2px 10px #0001;margin:10px 0}
    .card h2{font-size:.9rem;font-weight:600;margin:0 0 12px;color:#555;text-transform:uppercase;letter-spacing:.05em}
    .badge{display:inline-block;padding:3px 10px;border-radius:20px;font-size:.8rem;font-weight:bold}
    .on      {background:#d4edda;color:#155724}
    .off     {background:#f8d7da;color:#721c24}
    .flow    {background:#cce5ff;color:#004085}
    .override{background:#fff3cd;color:#856404}
    .row{display:flex;justify-content:space-between;align-items:center;margin:6px 0;font-size:.9rem}
    .label{color:#666}
    button{width:100%;padding:11px;margin-top:8px;font-size:.95rem;border:none;border-radius:8px;cursor:pointer;color:#fff;font-weight:500}
    .btn-on   {background:#28a745} .btn-on:hover  {background:#218838}
    .btn-off  {background:#dc3545} .btn-off:hover {background:#c82333}
    .btn-over {background:#e67e22} .btn-over:hover{background:#d35400}
    .btn-norm {background:#3498db} .btn-norm:hover{background:#2980b9}
    .toggle-row{display:flex;align-items:center;justify-content:space-between;margin:8px 0}
    .toggle{position:relative;width:46px;height:24px;flex-shrink:0}
    .toggle input{opacity:0;width:0;height:0}
    .slider{position:absolute;inset:0;background:#ccc;border-radius:24px;cursor:pointer;transition:.3s}
    .slider:before{content:"";position:absolute;width:18px;height:18px;left:3px;top:3px;background:#fff;border-radius:50%;transition:.3s}
    input:checked+.slider{background:#3498db}
    input:checked+.slider:before{transform:translateX(22px)}
    select{width:100%;padding:9px;border:1px solid #ddd;border-radius:8px;font-size:.9rem;background:#fff;margin-top:4px}
    .status-msg{font-size:.8rem;color:#888;margin-top:8px;min-height:16px}
    .uptime{font-size:.85rem;color:#444;margin-top:4px}
    footer{text-align:center;font-size:.72rem;color:#aaa;margin-top:20px;line-height:1.6}
    footer a{color:#aaa}
  </style>
</head>
<body>
  <h1>⚡ QNDFF <span style="font-size:.8rem;font-weight:400">v1.0</span></h1>
  <div class="sub">Quick 'n Dirty Flowmeter Firmware</div>

  <!-- Status -->
  <div class="card">
    <h2>Status</h2>
    <div class="row"><span class="label">Mode</span><span id="mode-badge" class="badge">…</span></div>
    <div class="row"><span class="label">Relay</span><span id="relay-badge" class="badge">…</span></div>
    <div class="row"><span class="label">Volume</span><span id="volume">…</span></div>
    <div class="row"><span class="label">Lifetime</span><span id="uptime" class="uptime">…</span></div>
  </div>

  <!-- Override -->
  <div class="card">
    <h2>Override Mode</h2>
    <button class="btn-over" onclick="setOverride(1)">Enter Override</button>
    <button class="btn-norm" onclick="setOverride(0)">Return to Flow Mode</button>
  </div>

  <!-- Manual relay -->
  <div class="card">
    <h2>Manual Relay <span style="font-size:.75rem;font-weight:400;color:#aaa">(override only)</span></h2>
    <button class="btn-on"  onclick="setRelay('on')">Turn ON</button>
    <button class="btn-off" onclick="setRelay('off')">Turn OFF</button>
  </div>

  <!-- Display settings -->
  <div class="card">
    <h2>Display Settings</h2>
    <div class="toggle-row">
      <span class="label">Units</span>
      <div style="display:flex;align-items:center;gap:8px;font-size:.85rem">
        <span>US (gal)</span>
        <label class="toggle">
          <input type="checkbox" id="unit-toggle" onchange="setUnits(this.checked)">
          <span class="slider"></span>
        </label>
        <span>Metric (L)</span>
      </div>
    </div>
    <div style="margin-top:12px">
      <div class="label" style="margin-bottom:4px">Display Language</div>
      <select id="lang-select" onchange="setLanguage(this.value)">
        <option value="en">English</option>
        <option value="uk">Ukrainian / Українська</option>
        <option value="zh">Chinese / 中文</option>
      </select>
    </div>
  </div>

  <div class="status-msg" id="status"></div>

  <footer>
    &copy; 2026 <a href="mailto:ntc@crotser.dev">N. T. Crotser</a> &mdash;
    <a href="https://github.com/ntcrotser" target="_blank">GitHub</a> &mdash;
    GPL v3 &mdash; QNDFF v1.0
  </footer>

  <script>
    async function refresh(){
      try{
        const r=await fetch('/api/status');
        const d=await r.json();
        const mb=document.getElementById('mode-badge');
        mb.textContent=d.mode==='FLOW'?'Flowmeter':'OVERRIDE';
        mb.className='badge '+(d.mode==='FLOW'?'flow':'override');
        const rb=document.getElementById('relay-badge');
        rb.textContent=d.relay;
        rb.className='badge '+(d.relay==='ON'?'on':'off');
        document.getElementById('volume').textContent=d.volume;
        document.getElementById('uptime').textContent=d.uptime;
        document.getElementById('unit-toggle').checked=(d.units==='metric');
        const ls=document.getElementById('lang-select');
        if(ls.value!==d.lang) ls.value=d.lang;
      }catch(e){}
    }
    async function setOverride(enable){
      await fetch('/api/override?enable='+enable,{method:'POST'});
      document.getElementById('status').textContent=enable?'Override enabled.':'Flow mode restored.';
      refresh();
    }
    async function setRelay(state){
      const r=await fetch('/api/relay?state='+state,{method:'POST'});
      document.getElementById('status').textContent=await r.text();
      refresh();
    }
    async function setUnits(metric){
      await fetch('/api/units?system='+(metric?'metric':'us'),{method:'POST'});
      document.getElementById('status').textContent='Units updated.';
      refresh();
    }
    async function setLanguage(lang){
      await fetch('/api/language?lang='+lang,{method:'POST'});
      document.getElementById('status').textContent='Language updated.';
    }
    refresh();
    setInterval(refresh,2000);
  </script>
</body>
</html>
)rawhtml";

// ── Route registration ────────────────────────────────────────────────────────
namespace WebInterface {

void registerRoutes(AsyncWebServer& server,
                    RelayController& relay,
                    Settings& settings,
                    Persistence& persistence,
                    uint32_t& serviceMinutes) {

    // GET / — control page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest* req) {
        req->send_P(200, "text/html", INDEX_HTML);
    });

    // GET /api/status — full JSON status
    server.on("/api/status", HTTP_GET, [&relay, &settings, &serviceMinutes](AsyncWebServerRequest* req) {
        const char* mode  = (relay.getMode()       == RelayController::Mode::FLOW) ? "FLOW"  : "OVERRIDE";
        const char* state = (relay.getRelayState()  == RelayController::State::ON) ? "ON"    : "OFF";
        const char* units = (settings.units == UnitSystem::US)                     ? "us"    : "metric";
        const char* lang  = (settings.language == Language::English)   ? "en" :
                            (settings.language == Language::Ukrainian)  ? "uk" : "zh";

        // Format volume
        float litres  = relay.getPulseCount() / FLOW_PULSES_PER_LITRE;
        char  volBuf[32];
        if (settings.units == UnitSystem::Metric) {
            snprintf(volBuf, sizeof(volBuf), "%.2f L", litres);
        } else {
            snprintf(volBuf, sizeof(volBuf), "%.2f gal", litres / LITRES_PER_GALLON);
        }

        // Format uptime
        char uptBuf[32];
        uint32_t m = serviceMinutes;
        if (m < 60)          snprintf(uptBuf, sizeof(uptBuf), "%lum",          (unsigned long)m);
        else if (m < 1440)   snprintf(uptBuf, sizeof(uptBuf), "%luh %lum",     (unsigned long)(m/60), (unsigned long)(m%60));
        else if (m < 43200)  snprintf(uptBuf, sizeof(uptBuf), "%lud %luh",     (unsigned long)(m/1440), (unsigned long)((m%1440)/60));
        else if (m < 525600) snprintf(uptBuf, sizeof(uptBuf), "%lumo %lud",    (unsigned long)(m/43200), (unsigned long)((m%43200)/1440));
        else                 snprintf(uptBuf, sizeof(uptBuf), "%luyr %lumo",   (unsigned long)(m/525600), (unsigned long)((m%525600)/43200));

        char buf[256];
        snprintf(buf, sizeof(buf),
                 "{\"mode\":\"%s\",\"relay\":\"%s\",\"pulses\":%lu,"
                 "\"volume\":\"%s\",\"uptime\":\"%s\","
                 "\"units\":\"%s\",\"lang\":\"%s\"}",
                 mode, state, (unsigned long)relay.getPulseCount(),
                 volBuf, uptBuf, units, lang);
        req->send(200, "application/json", buf);
    });

    // POST /api/override?enable=1|0
    server.on("/api/override", HTTP_POST, [&relay](AsyncWebServerRequest* req) {
        if (!req->hasParam("enable")) { req->send(400, "text/plain", "Missing: enable"); return; }
        bool enable = req->getParam("enable")->value() == "1";
        relay.setOverride(enable);
        req->send(200, "text/plain", enable ? "Override enabled" : "Flow mode restored");
    });

    // POST /api/relay?state=on|off
    server.on("/api/relay", HTTP_POST, [&relay](AsyncWebServerRequest* req) {
        if (!req->hasParam("state")) { req->send(400, "text/plain", "Missing: state"); return; }
        if (relay.getMode() != RelayController::Mode::OVERRIDE) {
            req->send(409, "text/plain", "Not in override mode"); return;
        }
        String val = req->getParam("state")->value();
        val.toLowerCase();
        if (val == "on") {
            relay.setRelayState(RelayController::State::ON);
            req->send(200, "text/plain", "Relay ON");
        } else if (val == "off") {
            relay.setRelayState(RelayController::State::OFF);
            req->send(200, "text/plain", "Relay OFF");
        } else {
            req->send(400, "text/plain", "state must be 'on' or 'off'");
        }
    });

    // POST /api/units?system=us|metric
    server.on("/api/units", HTTP_POST, [&settings, &persistence](AsyncWebServerRequest* req) {
        if (!req->hasParam("system")) { req->send(400, "text/plain", "Missing: system"); return; }
        String val = req->getParam("system")->value();
        settings.units = (val == "metric") ? UnitSystem::Metric : UnitSystem::US;
        persistence.saveSettings(settings);
        req->send(200, "text/plain", val == "metric" ? "Metric (litres)" : "US (gallons)");
    });

    // POST /api/language?lang=en|uk|zh
    server.on("/api/language", HTTP_POST, [&settings, &persistence](AsyncWebServerRequest* req) {
        if (!req->hasParam("lang")) { req->send(400, "text/plain", "Missing: lang"); return; }
        String val = req->getParam("lang")->value();
        if      (val == "uk") settings.language = Language::Ukrainian;
        else if (val == "zh") settings.language = Language::Chinese;
        else                  settings.language = Language::English;
        persistence.saveSettings(settings);
        req->send(200, "text/plain", "Language updated");
    });

    // 404
    server.onNotFound([](AsyncWebServerRequest* req) {
        req->send(404, "text/plain", "Not found");
    });
}

} // namespace WebInterface
