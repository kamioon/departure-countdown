#ifndef ALERTS_H
#define ALERTS_H

#include <Arduino.h>
#include "countdown_calc.h"
#include "config.h"


/**
 * @brief Alert type enumeration
 */
enum AlertType {
    ALERT_NONE = 0,
    ALERT_TIME_TO_LEAVE,    // It's time to leave
    ALERT_HURRY_UP,         // 5 min before departure
    ALERT_DEPARTING,        // 1 min before departure
    ALERT_DELAYED,          // Train delayed
    ALERT_CANCELLED         // Train cancelled
};

/**
 * @brief LED color enumeration
 */
enum LedColor {
    LED_OFF = 0,
    LED_GREEN,      // Safe
    LED_YELLOW,     // Get ready
    LED_ORANGE,     // Time to go
    LED_RED,        // Urgent
    LED_BLUE,       // Delayed
    LED_PURPLE      // No departures
};

/**
 * @brief Alert Manager
 *
 * Handles:
 * - Audio alerts (buzzer patterns)
 * - Visual alerts (LED blink patterns)
 * - Smart warnings based on countdown state
 * - Configurable alert thresholds
 */
class AlertManager {
public:
    AlertManager();
    ~AlertManager();

    /**
     * @brief Initialize alert hardware
     * @param config Configuration manager reference
     * @return true if initialization successful
     */
    bool begin(ConfigManager* config);

    /**
     * @brief Update alerts based on countdown state
     * @param info Countdown information
     * @param mode Transport mode for mode-specific alerts
     */
    void update(const CountdownInfo& info, TransportMode mode);

    /**
     * @brief Trigger specific alert
     * @param type Alert type
     * @param mode Transport mode for mode-specific alerts
     */
    void triggerAlert(AlertType type, TransportMode mode);

    /**
     * @brief Stop all alerts
     */
    void stopAll();

    /**
     * @brief Set LED color
     * @param color LED color
     * @param blink Enable blinking
     */
    void setLedColor(LedColor color, bool blink = false);

    /**
     * @brief Play buzzer tone
     * @param frequency Frequency in Hz
     * @param duration Duration in ms
     */
    void playTone(int frequency, int duration);

    /**
     * @brief Play alert pattern
     * @param type Alert type
     * @param mode Transport mode for mode-specific alerts
     */
    void playAlertPattern(AlertType type, TransportMode mode);

    /**
     * @brief Check if alert should be triggered
     * @param info Countdown information
     * @return Alert type to trigger, or ALERT_NONE
     */
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

    /**
     * @brief Initialize buzzer pin
     */
    void initBuzzer();

    /**
     * @brief Initialize LED pins
     */
    void initLeds();

    /**
     * @brief Update LED blinking
     */
    void updateBlink();

    /**
     * @brief Set RGB LED
     */
    void setRgbLed(int r, int g, int b);

    /**
     * @brief Get LED color for countdown state
     */
    LedColor getColorForState(CountdownState state);
};

#endif // ALERTS_H
