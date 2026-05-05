#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include "countdown_calc.h"
#include "config.h"

class MD_Parola;

enum DisplayMode {
    DISPLAY_COUNTDOWN = 0,
    DISPLAY_TRAIN_INFO,
    DISPLAY_STATUS,
    DISPLAY_ERROR
};

class DisplayManager {
public:
    DisplayManager();
    ~DisplayManager();

    bool begin();
    void update();
    void showCountdown(const CountdownInfo& info, TransportMode mode, bool flashTimer = false);
    void showTrainInfo(const Departure* departure);
    void showMessage(const String& message);
    void showError(const String& error);
    void clear();
    void setIntensity(uint8_t intensity);
    void showStartupAnimation();

private:
    MD_Parola* display;
    DisplayMode currentMode;
    String currentText;

    String formatCountdownText(const CountdownInfo& info, TransportMode mode);
    String formatTrainInfoText(const Departure* departure);
    char getModeIcon(TransportMode mode);
};

#endif // DISPLAY_H
