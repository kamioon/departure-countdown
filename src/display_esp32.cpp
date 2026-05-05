#include "display.h"
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include "pins.h"

// 8x8 Walking person icon for urgent state (Walk mode)
const uint8_t walkingPersonIcon[8] = {
    0b00011000,  // Head
    0b00011000,
    0b00111100,  // Body
    0b00011000,
    0b00011000,
    0b00101010,  // Legs walking
    0b01000100,
    0b10000010
};

// 8x8 Bicycle icon for urgent state (Bike mode)
const uint8_t bicycleIcon[8] = {
    0b00100010,  // Two wheels
    0b01010101,
    0b10101010,
    0b01000100,  // Frame
    0b00111000,  // Seat and handlebars
    0b00010000,
    0b00101000,
    0b01000100
};

DisplayManager::DisplayManager() :
    currentMode(DISPLAY_COUNTDOWN) {
    display = nullptr;
}

DisplayManager::~DisplayManager() {
    if (display) {
        delete display;
    }
}

bool DisplayManager::begin() {
    display = new MD_Parola(MD_MAX72XX::FC16_HW, MAX7219_CS, MAX_DEVICES);

    if (!display) {
        Serial.println("Failed to create display object");
        return false;
    }

    display->begin();
    display->setIntensity(5);  // Medium brightness
    display->displayClear();

    Serial.println("Display initialized");
    return true;
}

void DisplayManager::update() {
    if (!display) return;

    display->displayAnimate();
}

void DisplayManager::showCountdown(const CountdownInfo& info, TransportMode mode, bool flashTimer) {
    currentMode = DISPLAY_COUNTDOWN;

    if (!display) return;

    display->displayClear();

    // Use the pre-copied direction string — safe when departure pointer is null
    // (e.g. during a background fetch where we can't safely dereference it).
    String dirStr = info.departure ? info.departure->direction : info.direction;

    // Abbreviate to fit the display — take up to 4 chars of the first word
    String destination = dirStr.length() > 0 ? dirStr.substring(0, dirStr.indexOf(' ') > 0 ? min((int)dirStr.indexOf(' '), 4) : min((int)dirStr.length(), 4)) : "";

    // Format timer (middle)
    String timer = CountdownCalculator::formatCountdown(info.secondsUntilLeave);

    // Urgent = < 60 seconds
    bool isUrgent = (info.secondsUntilLeave < 60 && info.secondsUntilLeave > 0);

    // Mode icon (right, but not on last module)
    char modeIcon = getModeIcon(mode);

    // Build text: keep under 7 modules when urgent (rightmost reserved for icon).
    String displayText = destination;

    if (!isUrgent) {
        displayText += " ";
    }

    if (flashTimer) {
        displayText += "     ";  // Blank timer when flashing
    } else {
        displayText += timer;
    }

    if (!isUrgent) {
        displayText += " " + String(modeIcon);
    } else if (displayText.length() > 7) {
        displayText = displayText.substring(0, 7);
    }

    // Display text (LEFT aligned = DEST on module 0)
    display->setTextAlignment(PA_LEFT);
    display->print(displayText.c_str());

    // Draw icon on RIGHTMOST module (module 7) when urgent
    if (isUrgent && !flashTimer) {
        const uint8_t* icon = (mode == BIKE) ? bicycleIcon : walkingPersonIcon;

        MD_MAX72XX* hw = display->getGraphicObject();
        if (hw) {
            // NOTE: On this wiring, column 0 maps to the RIGHTMOST physical module.
            // If your module order is normal, flip ICON_RIGHTMOST_IS_COL0 to false.
            const bool ICON_RIGHTMOST_IS_COL0 = true;
            int startCol = ICON_RIGHTMOST_IS_COL0 ? 0 : (MAX_DEVICES - 1) * 8;

            for (int row = 0; row < 8; row++) {
                for (int col = 0; col < 8; col++) {
                    bool pixelOn = (icon[row] >> (7 - col)) & 0x01;
                    hw->setPoint(row, startCol + col, pixelOn);
                }
            }
        }
    }

    currentText = displayText;
}

void DisplayManager::showTrainInfo(const Departure* departure) {
    if (!departure) {
        showError("No departure data");
        return;
    }

    currentMode = DISPLAY_TRAIN_INFO;
    currentText = formatTrainInfoText(departure);

    if (!display) return;

    // Use scrolling for long text
    display->displayClear();
    display->displayScroll(currentText.c_str(), PA_LEFT, PA_SCROLL_LEFT, 50);
}

void DisplayManager::showMessage(const String& message) {
    currentMode = DISPLAY_STATUS;
    currentText = message;

    if (!display) return;

    display->displayClear();

    if (message.length() <= 8) {
        // Short message - just print
        display->setTextAlignment(PA_CENTER);
        display->print(message.c_str());
    } else {
        // Long message - scroll
        display->displayScroll(message.c_str(), PA_LEFT, PA_SCROLL_LEFT, 50);
    }
}

void DisplayManager::showError(const String& error) {
    currentMode = DISPLAY_ERROR;
    currentText = "ERR: " + error;

    if (!display) return;

    display->displayClear();
    display->displayScroll(currentText.c_str(), PA_LEFT, PA_SCROLL_LEFT, 50);
}

void DisplayManager::clear() {
    if (!display) return;
    display->displayClear();
    currentText = "";
}

void DisplayManager::setIntensity(uint8_t intensity) {
    if (!display) return;

    if (intensity > 15) intensity = 15;
    display->setIntensity(intensity);
}

void DisplayManager::showStartupAnimation() {
    if (!display) return;

    // Simple startup: show "DEPARTURE COUNTDOWN"
    display->displayClear();
    display->displayScroll("DEPARTURE COUNTDOWN", PA_LEFT, PA_SCROLL_LEFT, 50);

    // Wait for animation to complete
    while (!display->displayAnimate()) {
        delay(10);
    }

    delay(1000);
    display->displayClear();
}

String DisplayManager::formatCountdownText(const CountdownInfo& info, TransportMode mode) {
    String text = "L ";  // "L" for "Leave in"

    // Format countdown time
    text += CountdownCalculator::formatCountdown(info.secondsUntilLeave);

    // Add mode icon
    text += " ";
    text += getModeIcon(mode);

    return text;
}

String DisplayManager::formatTrainInfoText(const Departure* departure) {
    if (!departure) {
        return "NO DATA";
    }

    String text = "";

    // Direction
    text += departure->direction;

    // Train type
    if (departure->trainType.length() > 0) {
        text += " " + departure->trainType;
    }

    // Track
    if (departure->actualTrack.length() > 0) {
        text += " T" + departure->actualTrack;
    }

    // Cancelled?
    if (departure->cancelled) {
        text += " CANCELLED";
    }

    return text;
}

char DisplayManager::getModeIcon(TransportMode mode) {
    switch (mode) {
        case WALK:
            return 'W';
        case BIKE:
            return 'B';
        case BUS:
            return 'U';
        default:
            return '?';
    }
}
