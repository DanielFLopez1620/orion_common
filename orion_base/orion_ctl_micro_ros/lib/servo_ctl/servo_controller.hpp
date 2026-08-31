/**
 * @file servo_controller.hpp
 * @brief Servo arm control logic (position commands + feedback).
 *
 * Manages left and right servo motors for the ORION forward-looking arm.
 * Encapsulates servo positioning logic with angle clamping.
 */

#ifndef SERVO_CONTROLLER_HPP
#define SERVO_CONTROLLER_HPP

#include "servo.hpp"

/**
 * @class ServoController
 * @brief Manages servo motor control and feedback.
 *
 * Responsibilities:
 * - Accept position commands (radians) from external source (e.g., ROS)
 * - Command servos to desired positions
 * - Provide current servo positions for feedback publishing
 */
class ServoController {
private:
    fwd::ServoMotor servo_left;
    fwd::ServoMotor servo_right;

    // Servo angle limits (microseconds → ~30°–150°)
    static constexpr unsigned int MAX_POS = 150;
    static constexpr unsigned int MIN_POS = 30;

    // Offset: servos are centered at 90° (π/2 rad) at rest
    static constexpr float ANGLE_OFFSET = M_PI_2;

public:
    /**
     * @brief Constructor initializing servo pins.
     *
     * @param left_pin GPIO pin for left servo (PWM)
     * @param right_pin GPIO pin for right servo (PWM)
     */
    ServoController(unsigned int left_pin, unsigned int right_pin);

    /**
     * @brief Initializes both servos; must be called once in setup().
     */
    void initialize();

    /**
     * @brief Sets left servo to target position.
     *
     * @param radians Target position in radians (centered at 0, typical range [-π/2, π/2])
     */
    void setLeftPosition(float radians);

    /**
     * @brief Sets right servo to target position.
     *
     * @param radians Target position in radians (centered at 0, typical range [-π/2, π/2])
     */
    void setRightPosition(float radians);

    /**
     * @brief Returns current left servo position (radians, offset from π/2).
     */
    float getLeftPosition();

    /**
     * @brief Returns current right servo position (radians, offset from π/2).
     */
    float getRightPosition();
};

#endif
