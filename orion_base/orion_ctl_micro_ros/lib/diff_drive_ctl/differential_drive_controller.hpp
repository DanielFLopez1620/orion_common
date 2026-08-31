/**
 * @file differential_drive_controller.hpp
 * @brief Differential drive control logic (motor + encoder + PID feedback).
 *
 * Encapsulates all control logic for the ORION differential drive base:
 * - Motor command handling
 * - Encoder feedback reading
 * - PID speed regulation
 * - Command timeout detection
 *
 */

#ifndef DIFFERENTIAL_DRIVE_CONTROLLER_HPP
#define DIFFERENTIAL_DRIVE_CONTROLLER_HPP

#include <stdint.h>
#include "motor.hpp"
#include "encoder.hpp"
#include "pid.hpp"
#include "constants.hpp"

/**
 * @class DifferentialDriveController
 * @brief Manages differential drive motor control with PID feedback.
 *
 * Responsibilities:
 * - Accept motor speed commands from external source (e.g., ROS)
 * - Adjust motor speeds via PID feedback from encoders
 * - Detect command timeouts and stop motors safely
 * - Expose encoder feedback for odometry publishing
 */
class DifferentialDriveController {
private:
    // Hardware drivers (composition)
    diff::MotorDriver motor_left;
    diff::MotorDriver motor_right;
    diff::EncoderDriver enc_left;
    diff::EncoderDriver enc_right;
    diff::ControlPID pid_left;
    diff::ControlPID pid_right;

    // Command state
    bool received_cmd = false;
    unsigned long last_cmd_time = 0;
    static constexpr unsigned long TIMEOUT_MS = 1000;

    // Private methods
    void adjustMotorsSpeeds();

public:
    /**
     * @brief Constructor initializing all hardware pins.
     *
     * @param ml_en Motor left enable (PWM) pin
     * @param ml_fwd Motor left forward direction pin
     * @param ml_bwd Motor left backward direction pin
     * @param mr_en Motor right enable (PWM) pin
     * @param mr_fwd Motor right forward direction pin
     * @param mr_bwd Motor right backward direction pin
     * @param ml_enca Motor left encoder A pin
     * @param ml_encb Motor left encoder B pin
     * @param mr_enca Motor right encoder A pin
     * @param mr_encb Motor right encoder B pin
     */
    DifferentialDriveController(
        unsigned int ml_en, unsigned int ml_fwd, unsigned int ml_bwd,
        unsigned int mr_en, unsigned int mr_fwd, unsigned int mr_bwd,
        unsigned int ml_enca, unsigned int ml_encb,
        unsigned int mr_enca, unsigned int mr_encb
    );

    /**
     * @brief Initializes all motors and encoders; must be called once in setup().
     */
    void initialize();

    /**
     * @brief Safe startup: stops motors, resets encoders, homes PID controllers.
     */
    void safeStartup();

    /**
     * @brief Main update loop: computes PID and adjusts motor speeds.
     * Call once per loop() iteration.
     */
    void update();

    /**
     * @brief Checks for command timeout and stops motors if expired.
     * Call once per loop() iteration.
     */
    void checkTimeout();

    /**
     * @brief Sets target motor speeds from external command.
     *
     * @param left_speed Target speed for left motor (encoder tics/PID cycle; 0 to stop)
     * @param right_speed Target speed for right motor (encoder tics/PID cycle; 0 to stop)
     */
    void setTargetSpeed(int left_speed, int right_speed);

    /**
     * @brief Returns current left encoder count.
     *
     * Not const: the underlying EncoderDriver::read() mutates driver state.
     */
    int64_t getLeftEncoderCount();

    /**
     * @brief Returns current right encoder count.
     *
     * Not const: the underlying EncoderDriver::read() mutates driver state.
     */
    int64_t getRightEncoderCount();

    /**
     * @brief Called by ISR when left encoder edge detected.
     * Reads quadrature and updates encoder state.
     */
    void onLeftEncoderEdge();

    /**
     * @brief Called by ISR when right encoder edge detected.
     * Reads quadrature and updates encoder state.
     */
    void onRightEncoderEdge();
};

#endif
