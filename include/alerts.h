#ifndef ALERTS_H
#define ALERTS_H

#include <Arduino.h>
#include "countdown_calc.h"
#include "config.h"

enum AlertType {
    ALERT_NONE = 0,
    ALERT_TIME_TO_LEAVE,
    ALERT_HURRY_UP,
    ALERT_DEPARTING,
    ALERT_DELAYED,
    ALERT_CANCELLED
};

enum LedColor {
    LED_OFF = 0,
    LED_GREEN,    // safe
    LED_YELLOW,   // get ready
    LED_ORANGE,   // time to go
    LED_RED,      // urgent
    LED_BLUE,     // delayed
    LED_PURPLE    // no departures / error
};

class AlertManager {
public:
    AlertManager();
    ~AlertManager();

    bool begin(ConfigManager* config);
    void update(const CountdownInfo& info, TransportMode mode);
    void triggerAlert(AlertType type, TransportMode mode);
    void stopAll();
    void setLedColor(LedColor color, bool blink = false);
    void playTone(int frequency, int duration);
    void playAlertPattern(AlertType type, TransportMode mode);
    AlertType shouldTriggerAlert(const CountdownInfo& info);

private:
    ConfigManager* configManager;
    AlertType lastAlert;
    CountdownState lastState;
    unsigned long lastDepartureTime;
    unsigned long lastBlinkTime;
    bool blinkState;
    bool ledBlinking;
    LedColor currentLedColor;

    void initBuzzer();
    void initLeds();
    void updateBlink();
    void setRgbLed(int r, int g, int b);
    LedColor getColorForState(CountdownState state);
};

#endif // ALERTS_H
