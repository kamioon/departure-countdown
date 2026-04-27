#include "alerts.h"

AlertManager::AlertManager() :
    configManager(nullptr),
    lastAlert(ALERT_NONE),
    lastState(COUNTDOWN_SAFE),
    lastBlinkTime(0),
    blinkState(false),
    ledBlinking(false),
    currentLedColor(LED_OFF) {
}

AlertManager::~AlertManager() {
}

bool AlertManager::begin(ConfigManager* config) {
    configManager = config;
    return true;
}

void AlertManager::update(const CountdownInfo& info, TransportMode mode) {
    (void)info;
    (void)mode;
}

void AlertManager::triggerAlert(AlertType type, TransportMode mode) {
    lastAlert = type;
    (void)mode;
}

void AlertManager::stopAll() {
    lastAlert = ALERT_NONE;
}

void AlertManager::setLedColor(LedColor color, bool blink) {
    currentLedColor = color;
    ledBlinking = blink;
}

void AlertManager::playTone(int frequency, int duration) {
    (void)frequency;
    (void)duration;
}

void AlertManager::playAlertPattern(AlertType type, TransportMode mode) {
    lastAlert = type;
    (void)mode;
}

AlertType AlertManager::shouldTriggerAlert(const CountdownInfo& info) {
    (void)info;
    return ALERT_NONE;
}

void AlertManager::initBuzzer() {
}

void AlertManager::initLeds() {
}

void AlertManager::updateBlink() {
}

void AlertManager::setRgbLed(int r, int g, int b) {
    (void)r;
    (void)g;
    (void)b;
}

LedColor AlertManager::getColorForState(CountdownState state) {
    (void)state;
    return LED_OFF;
}
