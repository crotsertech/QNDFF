#include "RelayController.h"

RelayController::RelayController(uint8_t relayPin, uint32_t flowTimeoutMs)
    : _relayPin(relayPin), _flowTimeoutMs(flowTimeoutMs) {}

void RelayController::begin() {
    pinMode(_relayPin, OUTPUT);
    applyRelayState(State::OFF);
    Serial.printf("[Relay] Initialized on GPIO%d  flow_timeout=%lums\n",
                  _relayPin, (unsigned long)_flowTimeoutMs);
}

void RelayController::setInitialPulseCount(uint32_t count) {
    noInterrupts();
    _pulseCount = count;
    interrupts();
    Serial.printf("[Relay] Pulse count restored: %lu\n", (unsigned long)count);
}

void RelayController::update() {
    if (_mode != Mode::FLOW) return;

    // Snapshot volatile last-pulse time safely
    uint32_t lastPulse;
    noInterrupts();
    lastPulse = _lastPulseMs;
    interrupts();

    bool flowDetected = (lastPulse > 0) &&
                        ((millis() - lastPulse) < _flowTimeoutMs);

    State desired = flowDetected ? State::ON : State::OFF;
    if (desired != _relayState) {
        logTransition(
            _relayState == State::ON ? "ON" : "OFF",
            desired     == State::ON ? "ON" : "OFF"
        );
        applyRelayState(desired);
    }
}

void IRAM_ATTR RelayController::onFlowPulse() {
    _lastPulseMs = millis();
    ++_pulseCount;
}

void RelayController::setOverride(bool enable) {
    if (enable && _mode != Mode::OVERRIDE) {
        _mode = Mode::OVERRIDE;
        Serial.println("[Relay] Override mode ENABLED – flow detection paused");
    } else if (!enable && _mode != Mode::FLOW) {
        _mode = Mode::FLOW;
        // Reset pulse timestamp so we don't carry over a stale "flow active" state
        noInterrupts();
        _lastPulseMs = 0;
        interrupts();
        Serial.println("[Relay] Override mode DISABLED – resuming flow detection");
    }
}

void RelayController::setRelayState(State s) {
    if (_mode != Mode::OVERRIDE) {
        Serial.println("[Relay] Warning: setRelayState called outside override mode – ignored");
        return;
    }
    if (s != _relayState) {
        logTransition(
            _relayState == State::ON ? "ON" : "OFF",
            s           == State::ON ? "ON" : "OFF"
        );
        applyRelayState(s);
    }
}

// ── private ───────────────────────────────────────────────────────────────────

void RelayController::applyRelayState(State s) {
    _relayState = s;
    digitalWrite(_relayPin, s == State::ON ? HIGH : LOW);
}

void RelayController::logTransition(const char* from, const char* to) const {
    Serial.printf("[Relay] %s → %s  (mode=%s  pulses=%lu)\n",
                  from, to,
                  _mode == Mode::FLOW ? "FLOW" : "OVERRIDE",
                  (unsigned long)_pulseCount);
}
