#ifndef INPUT_H
#define INPUT_H

#include <Arduino.h>

class RotaryEncoder;

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

class InputHandler {
public:
    InputHandler();
    ~InputHandler();

    bool begin();
    void update();
    ButtonEvent getButtonEvent();

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

    const unsigned long LONG_PRESS_TIME = 1000;
    const unsigned long DEBOUNCE_TIME   = 50;

    void updateButtons();
    void updateEncoder();
    bool isButtonPressed(int pin);
};

#endif // INPUT_H
