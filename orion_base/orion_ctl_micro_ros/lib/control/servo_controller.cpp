/**
 * @file servo_controller.cpp
 * @brief Implementation of ServoController.
 */

#include "servo_controller.hpp"

ServoController::ServoController(unsigned int left_pin, unsigned int right_pin)
    : servo_left(MAX_POS, MIN_POS, left_pin),
      servo_right(MAX_POS, MIN_POS, right_pin) {}

void ServoController::initialize() {
    servo_left.begin();
    servo_right.begin();
}

void ServoController::setLeftPosition(float radians) {
    servo_left.setPositionRad(radians + ANGLE_OFFSET);
}

void ServoController::setRightPosition(float radians) {
    servo_right.setPositionRad(radians + ANGLE_OFFSET);
}

float ServoController::getLeftPosition() const {
    return servo_left.getPositionRad() - ANGLE_OFFSET;
}

float ServoController::getRightPosition() const {
    return servo_right.getPositionRad() - ANGLE_OFFSET;
}
