#include "input.h"

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
}

bool InputHandler::begin() {
    return true;
}

void InputHandler::update() {
}

void InputHandler::updateEncoder() {
}

void InputHandler::updateButtons() {
}

ButtonEvent InputHandler::getButtonEvent() {
    return BTN_NONE;
}

int InputHandler::getEncoderDelta() {
    return 0;
}

void InputHandler::resetEncoder() {
}

bool InputHandler::isButtonPressed(int pin) {
    (void)pin;
    return false;
}
