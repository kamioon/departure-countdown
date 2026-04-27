#include "display.h"

DisplayManager::DisplayManager() :
    currentMode(DISPLAY_COUNTDOWN),
    currentText(""),
    autoRotate(false),
    autoRotateInterval(0),
    lastRotateTime(0),
    rotateIndex(0) {
    display = nullptr;
}

DisplayManager::~DisplayManager() {
}

bool DisplayManager::begin() {
    return true;
}

void DisplayManager::update() {
}

void DisplayManager::showCountdown(const CountdownInfo& info, TransportMode mode, bool flashTimer) {
    (void)info;
    (void)mode;
    (void)flashTimer;
}

void DisplayManager::showTrainInfo(const Departure* departure) {
    (void)departure;
}

void DisplayManager::showMessage(const String& message) {
    currentText = message;
    currentMode = DISPLAY_STATUS;
}

void DisplayManager::showError(const String& error) {
    currentText = error;
    currentMode = DISPLAY_ERROR;
}

void DisplayManager::clear() {
    currentText = "";
}

void DisplayManager::setIntensity(uint8_t intensity) {
    (void)intensity;
}

void DisplayManager::setMode(DisplayMode mode) {
    currentMode = mode;
}

void DisplayManager::setAutoRotate(bool enabled, unsigned long intervalMs) {
    autoRotate = enabled;
    autoRotateInterval = intervalMs;
}

void DisplayManager::showStartupAnimation() {
}

String DisplayManager::formatCountdownText(const CountdownInfo& info, TransportMode mode) {
    (void)info;
    (void)mode;
    return "";
}

String DisplayManager::formatTrainInfoText(const Departure* departure) {
    (void)departure;
    return "";
}

char DisplayManager::getModeIcon(TransportMode mode) {
    (void)mode;
    return ' ';
}
