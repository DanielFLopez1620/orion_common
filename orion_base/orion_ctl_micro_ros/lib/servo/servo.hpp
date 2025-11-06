#ifndef SERVO_HPP
#define SERVO_HPP

#include <ESP32Servo.h>

namespace fwd
{
    /**
     * Class oriented to the management of servo motors by using the ESP32Servo
     * library.
     */
    class ServoMotor
    {
    private:
        unsigned int max_pos_{180}; // Max position of the servo
        unsigned int min_pos_{0};   // Min position of the servo
        unsigned int pwm_pin_{0};   // PWM pin of the servo
        float position_{0};         // Current position
        float objective_{0};        // Current objective
        Servo servo_;               // Servo object
    public:
        /**
         * User defined constructor to set up the max and min position of the
         * servo, and the PWM pin.
         *
         * @param max_pos Upper limit of servo movement
         * @param min_pos Lower limit of servo movement
         * @param pwm_pin Pin to attach the PWM contorl
         */
        ServoMotor(const unsigned int max_pos, const unsigned int min_pos, 
            const unsigned int pwm_pin)
            : max_pos_{max_pos}, min_pos_{min_pos}, pwm_pin_{pwm_pin} {}

        // Method prototypes

        void begin();

        void setPositionDeg(const float& degrees);

        float getPositionDeg();

        void setPositionRad(const float& radians);

        float getPositionRad();

        void approximatePositionDeg();

        void setObjectiveDeg(float degrees);

        void setObjectiveRad(float radians);

    }; // class ServoMotor

} // namespace fwd

#endif