#include "alerts.h"
#include "pins.h"

AlertManager::AlertManager() :
    configManager(nullptr),
    lastAlert(ALERT_NONE),
    lastState(COUNTDOWN_IDLE),
    lastDepartureTime(0),
    lastBlinkTime(0),
    blinkState(false),
    ledBlinking(false),
    currentLedColor(LED_OFF) {
}

AlertManager::~AlertManager() {
    stopAll();
}

bool AlertManager::begin(ConfigManager* config) {
    if (!config) {
        return false;
    }

    configManager = config;

    initBuzzer();
    initLeds();

    Serial.println("Alert manager initialized");
    return true;
}

void AlertManager::initBuzzer() {
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);
}

void AlertManager::initLeds() {
    pinMode(LED_R, OUTPUT);
    pinMode(LED_G, OUTPUT);
    pinMode(LED_B, OUTPUT);
    setRgbLed(0, 0, 0);  // Turn off
}

void AlertManager::update(const CountdownInfo& info, TransportMode mode) {
    if (!configManager) return;

    if (info.departureTime != 0 && info.departureTime != lastDepartureTime) {
        lastAlert = ALERT_NONE;
        lastDepartureTime = info.departureTime;
    }

    if (info.state != lastState) {
        LedColor color = getColorForState(info.state);
        bool shouldBlink = (info.state == COUNTDOWN_URGENT);
        setLedColor(color, shouldBlink);
    }

    AlertType alertToTrigger = shouldTriggerAlert(info);
    if (alertToTrigger != ALERT_NONE && alertToTrigger != lastAlert) {
        triggerAlert(alertToTrigger, mode);
        lastAlert = alertToTrigger;
    }

    updateBlink();

    lastState = info.state;
}

AlertType AlertManager::shouldTriggerAlert(const CountdownInfo& info) {
    if (!configManager || !configManager->getConfig().audioAlertsEnabled) {
        return ALERT_NONE;
    }

    // Alert when it's time to leave
    if (configManager->getConfig().alertAtLeaveTime &&
        info.state == COUNTDOWN_TIME_TO_GO &&
        lastState != COUNTDOWN_TIME_TO_GO) {
        return ALERT_TIME_TO_LEAVE;
    }

    // Alert 5 minutes before departure
    if (configManager->getConfig().alertFiveMinBefore &&
        info.secondsUntilDeparture <= 300 &&
        info.secondsUntilDeparture > 240 &&
        lastAlert != ALERT_HURRY_UP) {
        return ALERT_HURRY_UP;
    }

    // Alert 1 minute before leave time (more relevant for bike/walk).
    if (info.secondsUntilLeave <= 60 &&
        info.secondsUntilLeave > 0 &&
        lastAlert != ALERT_DEPARTING) {
        return ALERT_DEPARTING;
    }

    return ALERT_NONE;
}

void AlertManager::triggerAlert(AlertType type, TransportMode mode) {
    Serial.print("Triggering alert: ");
    Serial.println((int)type);

    playAlertPattern(type, mode);
}

void AlertManager::playAlertPattern(AlertType type, TransportMode mode) {
    if (!configManager || !configManager->getConfig().audioAlertsEnabled) {
        return;
    }

    switch (type) {
        case ALERT_TIME_TO_LEAVE:
            // Single long beep
            playTone(1000, 500);
            break;

        case ALERT_HURRY_UP:
            // Double beep
            playTone(1200, 200);
            delay(100);
            playTone(1200, 200);
            break;

        case ALERT_DEPARTING:
            // Mode-specific beeps (< 1 min to leave).
            if (mode == BIKE) {
                for (int i = 0; i < 2; i++) {
                    playTone(2000, 400);
                    delay(100);
                }
            } else {
                for (int i = 0; i < 2; i++) {
                    playTone(2000, 150);
                    delay(100);
                }
            }
            noTone(BUZZER_PIN);
            digitalWrite(BUZZER_PIN, HIGH);
            delay(250);
            digitalWrite(BUZZER_PIN, LOW);
            break;

        case ALERT_DELAYED:
            // Descending tone
            playTone(1000, 200);
            playTone(800, 200);
            break;

        case ALERT_CANCELLED:
            // Error tone
            playTone(500, 300);
            break;

        default:
            break;
    }
}

void AlertManager::playTone(int frequency, int duration) {
    tone(BUZZER_PIN, frequency, duration);
    delay(duration);
    noTone(BUZZER_PIN);
}

void AlertManager::stopAll() {
    noTone(BUZZER_PIN);
    setRgbLed(0, 0, 0);
    ledBlinking = false;
    currentLedColor = LED_OFF;
}

void AlertManager::setLedColor(LedColor color, bool blink) {
    if (!configManager || !configManager->getConfig().ledAlertsEnabled) {
        return;
    }

    currentLedColor = color;
    ledBlinking = blink;
    blinkState = true;

    if (!blink) {
        updateBlink();
    }
}

void AlertManager::updateBlink() {
    if (!configManager || !configManager->getConfig().ledAlertsEnabled) {
        return;
    }

    if (ledBlinking) {
        unsigned long now = millis();
        if (now - lastBlinkTime >= 500) {
            blinkState = !blinkState;
            lastBlinkTime = now;
        }
    }

    if (blinkState) {
        switch (currentLedColor) {
            case LED_GREEN:
                setRgbLed(0, 255, 0);
                break;
            case LED_YELLOW:
                setRgbLed(255, 255, 0);
                break;
            case LED_ORANGE:
                setRgbLed(255, 128, 0);
                break;
            case LED_RED:
                setRgbLed(255, 0, 0);
                break;
            case LED_BLUE:
                setRgbLed(0, 0, 255);
                break;
            case LED_PURPLE:
                setRgbLed(128, 0, 128);
                break;
            default:
                setRgbLed(0, 0, 0);
                break;
        }
    } else {
        setRgbLed(0, 0, 0);
    }
}

void AlertManager::setRgbLed(int r, int g, int b) {
    analogWrite(LED_R, r);
    analogWrite(LED_G, g);
    analogWrite(LED_B, b);
}

LedColor AlertManager::getColorForState(CountdownState state) {
    switch (state) {
        case COUNTDOWN_SAFE:
            return LED_GREEN;
        case COUNTDOWN_READY:
            return LED_YELLOW;
        case COUNTDOWN_TIME_TO_GO:
            return LED_ORANGE;
        case COUNTDOWN_URGENT:
            return LED_RED;
        case COUNTDOWN_DEPARTED:
            return LED_OFF;
        case COUNTDOWN_ERROR:
            return LED_PURPLE;
        default:
            return LED_OFF;
    }
}
