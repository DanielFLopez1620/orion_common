/*
 * @file servo.hpp
 * @brief Servo motor driver for ORION arm positioning.
 *
 * Wraps the ESP32Servo library to provide degree and radian interfaces
 * with configurable min/max angle limits for the left and right arm servos.
 */

#ifndef SERVO_HPP
#define SERVO_HPP

#include <ESP32Servo.h>

namespace fwd
{
    /*
     * Servo motor control wrapper using ESP32Servo library.
     * Provides degree/radian interfaces with position clamping and objectives.
     */
    class ServoMotor
    {
    private:
        unsigned int max_pos_{180};  // Max servo position (degrees)
        unsigned int min_pos_{0};    // Min servo position (degrees)
        unsigned int pwm_pin_{0};    // GPIO pin for servo PWM control
        float position_{0};          // Current servo position (degrees)
        float objective_{0};         // Target position for smooth interpolation
        Servo servo_;                // ESP32Servo object

    public:
        /*
         * Constructor that sets servo motion limits and PWM pin.
         * @param max_pos Upper position limit in degrees (0-180 typical)
         * @param min_pos Lower position limit in degrees (0-180 typical)
         * @param pwm_pin GPIO pin for servo PWM signal
         */
        ServoMotor(const unsigned int max_pos, const unsigned int min_pos,
            const unsigned int pwm_pin)
            : max_pos_{max_pos}, min_pos_{min_pos}, pwm_pin_{pwm_pin} {}

        /*
         * Attaches servo to PWM pin and initializes.
         * Call once during setup.
         */
        void begin();

        /*
         * Sets servo position with clamping to [min_pos_, max_pos_].
         * @param degrees Target position in degrees (0-180)
         */
        void setPositionDeg(const float& degrees);

        /*
         * Reads current servo position.
         * @return Current position in degrees (0-180)
         */
        float getPositionDeg();

        /*
         * Sets servo position from radians, converts to degrees.
         * @param radians Target position in radians (~[-π/2, π/2] for centered servo)
         */
        void setPositionRad(const float& radians);

        /*
         * Reads current servo position in radians.
         * @return Current position in radians
         */
        float getPositionRad();

        /*
         * Incrementally moves servo toward objective position.
         * Used for smooth interpolation when called repeatedly.
         */
        void approximatePositionDeg();

        /*
         * Sets interpolation target (objective) for approximatePositionDeg.
         * @param degrees Target position in degrees
         */
        void setObjectiveDeg(float degrees);

        /*
         * Sets interpolation target from radians.
         * @param radians Target position in radians
         */
        void setObjectiveRad(float radians);

    }; // class ServoMotor

} // namespace fwd

#endif