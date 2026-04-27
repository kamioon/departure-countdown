#ifndef INPUT_H
#define INPUT_H

#include <Arduino.h>

class RotaryEncoder;

/**
 * @brief Button event enumeration
 */
enum ButtonEvent {
    BTN_NONE = 0,
    BTN_START_PRESS,
    BTN_START_LONG_PRESS,
    BTN_RESET_PRESS,
    BTN_RESET_LONG_PRESS,
    BTN_MODE_PRESS,
    BTN_MODE_LONG_PRESS,
    BTN_ENCODER_PRESS,
    BTN_ENCODER_LONG_PRESS
};

/**
 * @brief Input Handler
 *
 * Handles:
 * - Rotary encoder reading with debounce
 * - Button state detection (short/long press)
 * - Input event queue management
 */
class InputHandler {
public:
    InputHandler();
    ~InputHandler();

    /**
     * @brief Initialize input hardware
     * @return true if initialization successful
     */
    bool begin();

    /**
     * @brief Update input states (call in loop)
     */
    void update();

    /**
     * @brief Get next button event
     * @return Button event
     */
    ButtonEvent getButtonEvent();

    /**
     * @brief Get encoder position change
     * @return Position delta since last call
     */
    int getEncoderDelta();

    /**
     * @brief Reset encoder position
     */
    void resetEncoder();

private:
    RotaryEncoder* encoder;

    int lastEncoderPos;
    unsigned long startBtnPressTime;
    unsigned long resetBtnPressTime;
    unsigned long modeBtnPressTime;
    unsigned long encoderBtnPressTime;

    bool startBtnPressed;
    bool resetBtnPressed;
    bool modeBtnPressed;
    bool encoderBtnPressed;

    bool startBtnHandled;
    bool resetBtnHandled;
    bool modeBtnHandled;
    bool encoderBtnHandled;

    const unsigned long LONG_PRESS_TIME = 1000;  // 1 second
    const unsigned long DEBOUNCE_TIME = 50;      // 50 ms

    /**
     * @brief Update button state
     */
    void updateButtons();

    /**
     * @brief Update encoder state
     */
    void updateEncoder();

    /**
     * @brief Check button press
     */
    bool isButtonPressed(int pin);
};

#endif // INPUT_H
