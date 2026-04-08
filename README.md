# QNDFF — Quick 'n Dirty Flowmeter Firmware

**v1.0** · ESP32 · Arduino / PlatformIO · GPL v3

> Flow-detection relay control with OLED display, multilingual UI, WiFi web panel, and NVS-persisted service life tracking — all in one tidy firmware package.

---

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Hardware](#hardware)
  - [Bill of Materials](#bill-of-materials)
  - [Wiring / Pinout](#wiring--pinout)
- [Build & Flash](#build--flash)
  - [Prerequisites](#prerequisites)
  - [Clone & Build](#clone--build)
  - [Flash](#flash)
- [Web Panel](#web-panel)
- [OLED Display](#oled-display)
- [Configuration](#configuration)
- [License](#license)
- [Author](#author)

---

## Overview

QNDFF is a compact, open-source firmware for ESP32-based flow monitoring and relay control. It reads pulse output from a hall-effect flow sensor via hardware interrupt, drives a relay based on detected flow, and provides real-time feedback on a 128×64 OLED. A WiFi access point hosts a lightweight web panel for runtime control and configuration.

---

## Features

- **Interrupt-driven flow detection** — FALLING-edge ISR on the YF-B10 sensor; 300 ms timeout to declare flow stopped
- **Relay control modes**
  - **Flowswitch** — relay closes on flow, opens when flow stops
  - **Override** — manual relay control independent of sensor input; indicated by a λ (lambda) icon on the OLED
- **SSD1306 OLED display**
  - Scrolling `>->->->->->` animation while flow is active; static `----------` at rest
  - Relay state (OPEN / CLOSED), mode, and metered volume
  - Persisted service life counter (auto-formats: minutes → hours → days → months → years)
- **Multilingual display** — English, Ukrainian (Cyrillic), and Chinese (CJK) via U8g2_for_Adafruit_GFX
- **WiFi access point** — SSID `RelayControl` / password `12345678`; web panel at `192.168.4.1`
- **NVS persistence** — language, unit system, and service minutes survive reboots and power cycles; flushed to flash every 60 seconds
- **Boot splash** — firmware name, version, author, and license displayed for 2.5 s on startup
- **Unit toggle** — US gallons or metric litres (YF-B10 calibrated at ~5.5 pulses/litre)

---

## Hardware

### Bill of Materials

| Component | Model | Notes |
|---|---|---|
| Microcontroller | ESP32 Dev Board (30-pin) | 38-pin boards will not fit the expansion board; direct wire |
| Flow sensor | YF-B10 hall-effect | 5 V, interrupt output |
| Relay module | KY-019 | 5 V coil, CO / + / − terminals |
| OLED display | SSD1306 0.96″ 128×64 | I²C, 3.3 V |

### Wiring / Pinout

```
ESP32 Pin   →   Component
─────────────────────────────────────────────
GPIO 18     →   YF-B10 signal (yellow wire)
GPIO 21     →   SSD1306 SDA
GPIO 22     →   SSD1306 SCL
GPIO 26     →   KY-019 IN

VIN  (5 V)  →   YF-B10 VCC (red wire)
VIN  (5 V)  →   KY-019 + (VCC)
3V3         →   SSD1306 VCC
GND         →   YF-B10 GND (black wire)
GND         →   KY-019 − (GND)
GND         →   SSD1306 GND
```

> **Note:** GPIO 18 is configured `INPUT_PULLUP` with a FALLING-edge interrupt. The YF-B10 open-collector output is compatible with the internal pull-up; no external resistor is required.

---

## Build & Flash

### Prerequisites

- [PlatformIO Core](https://docs.platformio.org/en/latest/core/installation/index.html) (CLI) **or** [PlatformIO IDE](https://platformio.org/platformio-ide) (VS Code extension)
- USB cable + appropriate driver for your ESP32 board (CP210x or CH340)

Dependencies are declared in `platformio.ini` and resolved automatically on first build:

| Library | Purpose |
|---|---|
| `adafruit/Adafruit SSD1306` | OLED driver |
| `adafruit/Adafruit GFX Library` | Graphics primitives |
| `olikraus/U8g2_for_Adafruit_GFX` | Cyrillic & CJK font rendering |
| ESP32 Preferences (built-in) | NVS persistence |
| ESP32 WebServer (built-in) | WiFi AP + HTTP server |

### Clone & Build

```bash
git clone https://github.com/ntcrotser/QNDFF.git
cd QNDFF
pio run
```

### Flash

```bash
pio run --target upload
```

Monitor serial output (115200 baud):

```bash
pio device monitor --baud 115200
```

The boot log will confirm sensor, relay, display, WiFi AP, and NVS initialisation.

---

## Web Panel

Connect to the `RelayControl` WiFi network (password: `12345678`), then open a browser to:

```
http://192.168.4.1
```

### Panel Sections

| Section | Description |
|---|---|
| **Status** | Live mode badge, relay state, metered volume, and service uptime |
| **Override Mode** | Enter / exit Override; manually switch relay ON or OFF (override only) |
| **Display Settings** | Toggle between US (gal) and Metric (L); select display language |

All settings are applied immediately and persisted to NVS — they survive power cycles.

---

## OLED Display

The 128×64 display is divided into four rows:

```
Row 1  [animation]   >->->->->->   (flow active)
                     -----------   (no flow)
Row 2  Relay :       OPEN / CLOSED
Row 3  Mode  :       Flowswitch / OVERRIDE  [λ icon in Override]
Row 4  Volume:       12.47 Gal  /  47.20 L
Row 5  Svc:          5d 3h
```

### Language Rendering

| Language | Font used for labels | Value rendering |
|---|---|---|
| English | Default GFX ASCII | Same font |
| Ukrainian | `u8g2_font_6x13_t_cyrillic` | Hybrid: Cyrillic prefix + ASCII GFX for dynamic values |
| Chinese | `u8g2_font_wqy12_t_gb2312` | Hybrid: CJK prefix + ASCII GFX for dynamic values |

Hybrid rendering (native-script prefix, ASCII fallback for numeric values) prevents display overflow on the 128 px wide panel.

---

## Configuration

Key constants are defined in `src/Config.h`:

```cpp
// WiFi
#define WIFI_SSID        "RelayControl"
#define WIFI_PASSWORD    "12345678"

// Flow sensor
#define FLOW_SENSOR_PIN  18
#define FLOW_TIMEOUT_MS  300

// Relay
#define RELAY_PIN        26

// OLED
#define OLED_SDA         21
#define OLED_SCL         22

// Calibration (YF-B10, ~5.5 pulses/litre)
#define PULSES_PER_LITRE 5.5f
#define LITRES_PER_GALLON 3.78541f

// NVS save interval
#define PERSIST_INTERVAL_MS 60000
```

---

## License

QNDFF is free software, distributed under the [GNU General Public License v3.0](LICENSE).

```
Copyright (C) 2026  N. T. Crotser <ntc@crotser.dev>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
```

---

## Author

**N. T. Crotser**
[ntc@crotser.dev](mailto:ntc@crotser.dev) · [github.com/ntcrotser](https://github.com/ntcrotser)
