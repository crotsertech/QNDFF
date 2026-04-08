#pragma once
#include <Arduino.h>

/**
 * RelayController
 *
 * Manages the KY-019 relay with two modes:
 *
 *   FLOW     – Relay mirrors flow state: ON while pulses are detected from the
 *              YF-S201, OFF when no pulse arrives within FLOW_TIMEOUT_MS.
 *              A hardware interrupt on the sensor pin calls onFlowPulse().
 *
 *   OVERRIDE – Manual state held via WiFi command; flow detection paused.
 *
 * Call update() from loop() to tick the timeout check.
 */
class RelayController {
public:
    enum class Mode  { FLOW, OVERRIDE };
    enum class State { ON, OFF };

    RelayController(uint8_t relayPin, uint32_t flowTimeoutMs);

    void begin();
    void update();              // must be called every loop iteration

    // Restore persisted pulse count on boot
    void setInitialPulseCount(uint32_t count);

    // Called from ISR — marks the timestamp of the latest pulse
    void IRAM_ATTR onFlowPulse();

    // Override mode API (called from web handlers)
    void setOverride(bool enable);
    void setRelayState(State s);    // only meaningful in OVERRIDE mode

    // Accessors
    Mode     getMode()        const { return _mode; }
    State    getRelayState()  const { return _relayState; }
    uint32_t getPulseCount()  const { return _pulseCount; }

private:
    void applyRelayState(State s);
    void logTransition(const char* from, const char* to) const;

    uint8_t  _relayPin;
    uint32_t _flowTimeoutMs;

    Mode     _mode        = Mode::FLOW;
    State    _relayState  = State::OFF;

    volatile uint32_t _lastPulseMs  = 0;
    volatile uint32_t _pulseCount   = 0;    // lifetime pulse counter
};
