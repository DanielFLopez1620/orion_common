/**
 * @file differential_drive_controller.cpp
 * @brief Implementation of DifferentialDriveController.
 */

#include "differential_drive_controller.hpp"

DifferentialDriveController::DifferentialDriveController(
    unsigned int ml_en, unsigned int ml_fwd, unsigned int ml_bwd,
    unsigned int mr_en, unsigned int mr_fwd, unsigned int mr_bwd,
    unsigned int ml_enca, unsigned int ml_encb,
    unsigned int mr_enca, unsigned int mr_encb)
    : motor_left(ml_en, ml_fwd, ml_bwd),
      motor_right(mr_en, mr_fwd, mr_bwd),
      enc_left(ml_enca, ml_encb),
      enc_right(mr_enca, mr_encb),
      pid_left(
          diff::ROBOT_CONST::PID_KP, diff::ROBOT_CONST::PID_KD, diff::ROBOT_CONST::PID_KI,
          diff::ROBOT_CONST::PID_KO, diff::ROBOT_CONST::PWM_MAX, diff::ROBOT_CONST::PWM_MIN),
      pid_right(
          diff::ROBOT_CONST::PID_KP, diff::ROBOT_CONST::PID_KD, diff::ROBOT_CONST::PID_KI,
          diff::ROBOT_CONST::PID_KO, diff::ROBOT_CONST::PWM_MAX, diff::ROBOT_CONST::PWM_MIN) {}

void DifferentialDriveController::initialize() {
    motor_left.safeInit();
    motor_right.safeInit();

    motor_left.begin();
    motor_right.begin();
    pid_left.disable();
    pid_right.disable();
    pid_left.setSetpoint(0);
    pid_right.setSetpoint(0);

    enc_left.begin();
    enc_right.begin();
}

void DifferentialDriveController::safeStartup() {
    motor_left.setSpeed(0);
    motor_right.setSpeed(0);
    pid_left.disable();
    pid_right.disable();

    delay(500);
    enc_left.reset();
    enc_right.reset();

    pid_left.reset(enc_left.read());
    pid_right.reset(enc_right.read());

    delay(500);
}

void DifferentialDriveController::update() {
    adjustMotorsSpeeds();
}

void DifferentialDriveController::checkTimeout() {
    if (received_cmd && (millis() - last_cmd_time > TIMEOUT_MS)) {
        motor_left.setSpeed(0);
        motor_right.setSpeed(0);
        pid_left.disable();
        pid_right.disable();
        received_cmd = false;
    }
}

void DifferentialDriveController::setTargetSpeed(int left_speed, int right_speed) {
    if (left_speed == 0) {
        motor_left.setSpeed(0);
        pid_left.disable();
    } else {
        pid_left.enable();
    }

    if (right_speed == 0) {
        motor_right.setSpeed(0);
        pid_right.disable();
    } else {
        pid_right.enable();
    }

    pid_left.setSetpoint((float)left_speed / (float)diff::ROBOT_CONST::PID_RATE);
    pid_right.setSetpoint((float)right_speed / (float)diff::ROBOT_CONST::PID_RATE);

    received_cmd = true;
    last_cmd_time = millis();
}

int64_t DifferentialDriveController::getLeftEncoderCount() const {
    return enc_left.read();
}

int64_t DifferentialDriveController::getRightEncoderCount() const {
    return enc_right.read();
}

void DifferentialDriveController::onLeftEncoderEdge() {
    enc_left.readEnc();
}

void DifferentialDriveController::onRightEncoderEdge() {
    enc_right.readEnc();
}

void DifferentialDriveController::adjustMotorsSpeeds() {
    if (!received_cmd) return;

    int motor_left_sp = 0;
    int motor_right_sp = 0;

    pid_left.compute(enc_left.read(), motor_left_sp);
    pid_right.compute(enc_right.read(), motor_right_sp);

    if (pid_left.enabled()) motor_left.setSpeed(motor_left_sp);
    if (pid_right.enabled()) motor_right.setSpeed(motor_right_sp);
}
