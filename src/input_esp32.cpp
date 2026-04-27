#include "input.h"
#include <RotaryEncoder.h>
#include "pins.h"

InputHandler::InputHandler() :
    lastEncoderPos(0),
    startBtnPressTime(0),
    resetBtnPressTime(0),
    modeBtnPressTime(0),
    encoderBtnPressTime(0),
    startBtnPressed(false),
    resetBtnPressed(false),
    modeBtnPressed(false),
    encoderBtnPressed(false),
    startBtnHandled(false),
    resetBtnHandled(false),
    modeBtnHandled(false),
    encoderBtnHandled(false) {
    encoder = nullptr;
}

InputHandler::~InputHandler() {
    if (encoder) {
        delete encoder;
    }
}

bool InputHandler::begin() {
    // Initialize encoder
    encoder = new RotaryEncoder(ENC_DT, ENC_CLK, RotaryEncoder::LatchMode::TWO03);

    if (!encoder) {
        Serial.println("Failed to create encoder object");
        return false;
    }

    // Initialize button pins with pull-ups
    pinMode(BTN_START, INPUT_PULLUP);
    pinMode(BTN_RESET, INPUT_PULLUP);
    pinMode(BTN_MODE, INPUT_PULLUP);
    pinMode(ENC_SW, INPUT_PULLUP);

    Serial.println("Input handler initialized");
    return true;
}

void InputHandler::update() {
    updateEncoder();
    updateButtons();
}

void InputHandler::updateEncoder() {
    if (!encoder) return;
    encoder->tick();
}

void InputHandler::updateButtons() {
    unsigned long now = millis();

    // Start button
    bool startNow = isButtonPressed(BTN_START);
    if (startNow && !startBtnPressed) {
        startBtnPressed = true;
        startBtnPressTime = now;
        startBtnHandled = false;
    } else if (!startNow && startBtnPressed) {
        startBtnPressed = false;
    }

    // Reset button
    bool resetNow = isButtonPressed(BTN_RESET);
    if (resetNow && !resetBtnPressed) {
        resetBtnPressed = true;
        resetBtnPressTime = now;
        resetBtnHandled = false;
    } else if (!resetNow && resetBtnPressed) {
        resetBtnPressed = false;
    }

    // Mode button
    bool modeNow = isButtonPressed(BTN_MODE);
    if (modeNow && !modeBtnPressed) {
        modeBtnPressed = true;
        modeBtnPressTime = now;
        modeBtnHandled = false;
    } else if (!modeNow && modeBtnPressed) {
        modeBtnPressed = false;
    }

    // Encoder button
    bool encNow = isButtonPressed(ENC_SW);
    if (encNow && !encoderBtnPressed) {
        encoderBtnPressed = true;
        encoderBtnPressTime = now;
        encoderBtnHandled = false;
    } else if (!encNow && encoderBtnPressed) {
        encoderBtnPressed = false;
    }
}

ButtonEvent InputHandler::getButtonEvent() {
    unsigned long now = millis();

    // Check for long presses first
    if (startBtnPressed && !startBtnHandled &&
        now - startBtnPressTime >= LONG_PRESS_TIME) {
        startBtnHandled = true;
        return BTN_START_LONG_PRESS;
    }

    if (resetBtnPressed && !resetBtnHandled &&
        now - resetBtnPressTime >= LONG_PRESS_TIME) {
        resetBtnHandled = true;
        return BTN_RESET_LONG_PRESS;
    }

    if (modeBtnPressed && !modeBtnHandled &&
        now - modeBtnPressTime >= LONG_PRESS_TIME) {
        modeBtnHandled = true;
        return BTN_MODE_LONG_PRESS;
    }

    if (encoderBtnPressed && !encoderBtnHandled &&
        now - encoderBtnPressTime >= LONG_PRESS_TIME) {
        encoderBtnHandled = true;
        return BTN_ENCODER_LONG_PRESS;
    }

    // Check for short presses (button released)
    if (!startBtnPressed && !startBtnHandled &&
        startBtnPressTime > 0 &&
        now - startBtnPressTime < LONG_PRESS_TIME) {
        startBtnPressTime = 0;
        return BTN_START_PRESS;
    }

    if (!resetBtnPressed && !resetBtnHandled &&
        resetBtnPressTime > 0 &&
        now - resetBtnPressTime < LONG_PRESS_TIME) {
        resetBtnPressTime = 0;
        return BTN_RESET_PRESS;
    }

    if (!modeBtnPressed && !modeBtnHandled &&
        modeBtnPressTime > 0 &&
        now - modeBtnPressTime < LONG_PRESS_TIME) {
        modeBtnPressTime = 0;
        return BTN_MODE_PRESS;
    }

    if (!encoderBtnPressed && !encoderBtnHandled &&
        encoderBtnPressTime > 0 &&
        now - encoderBtnPressTime < LONG_PRESS_TIME) {
        encoderBtnPressTime = 0;
        return BTN_ENCODER_PRESS;
    }

    return BTN_NONE;
}

int InputHandler::getEncoderDelta() {
    if (!encoder) return 0;

    int newPos = encoder->getPosition();
    int delta = newPos - lastEncoderPos;
    lastEncoderPos = newPos;
    return delta;
}

void InputHandler::resetEncoder() {
    if (!encoder) return;
    encoder->setPosition(0);
    lastEncoderPos = 0;
}

bool InputHandler::isButtonPressed(int pin) {
    return digitalRead(pin) == LOW;  // Pull-up resistors, so LOW when pressed
}
