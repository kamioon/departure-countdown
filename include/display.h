#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include "countdown_calc.h"
#include "config.h"

class MD_Parola;

/**
 * @brief Display mode enumeration
 */
enum DisplayMode {
    DISPLAY_COUNTDOWN = 0,      // Show countdown timer
    DISPLAY_TRAIN_INFO,         // Show train information
    DISPLAY_STATUS,             // Show status message
    DISPLAY_ERROR               // Show error message
};

/**
 * @brief Display Manager for MAX7219 LED Matrix
 *
 * Handles:
 * - Initialize display hardware (MAX7219/LED Matrix)
 * - Render countdown time (MM:SS format)
 * - Show next train info (destination, track, delay)
 * - Display travel method icon
 * - Scrolling messages on LED matrix
 * - Handle screen updates efficiently
 */
class DisplayManager {
public:
    DisplayManager();
    ~DisplayManager();

    /**
     * @brief Initialize display
     * @return true if initialization successful
     */
    bool begin();

    /**
     * @brief Update display (call in loop)
     */
    void update();

    /**
     * @brief Show countdown information
     * @param info Countdown information
     * @param mode Transport mode
     * @param flashTimer Flash only the timer (not entire display)
     */
    void showCountdown(const CountdownInfo& info, TransportMode mode, bool flashTimer = false);

    /**
     * @brief Show train information
     * @param departure Departure information
     */
    void showTrainInfo(const Departure* departure);

    /**
     * @brief Show scrolling message
     * @param message Message to display
     */
    void showMessage(const String& message);

    /**
     * @brief Show error message
     * @param error Error message
     */
    void showError(const String& error);

    /**
     * @brief Clear display
     */
    void clear();

    /**
     * @brief Set display intensity
     * @param intensity Brightness (0-15)
     */
    void setIntensity(uint8_t intensity);

    /**
     * @brief Set display mode
     * @param mode Display mode
     */
    void setMode(DisplayMode mode);

    /**
     * @brief Enable/disable auto-rotate modes
     * @param enabled true to auto-rotate
     * @param intervalMs Interval in milliseconds
     */
    void setAutoRotate(bool enabled, unsigned long intervalMs = 5000);

    /**
     * @brief Show startup animation
     */
    void showStartupAnimation();

private:
    MD_Parola* display;

    DisplayMode currentMode;
    String currentText;
    bool autoRotate;
    unsigned long autoRotateInterval;
    unsigned long lastRotateTime;
    int rotateIndex;

    /**
     * @brief Format countdown display text
     */
    String formatCountdownText(const CountdownInfo& info, TransportMode mode);

    /**
     * @brief Format train info text
     */
    String formatTrainInfoText(const Departure* departure);

    /**
     * @brief Get mode icon character
     */
    char getModeIcon(TransportMode mode);
};

#endif // DISPLAY_H
