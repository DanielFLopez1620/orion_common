// //////////////////////// Include Libraries //////////////////////////////
// -------------------- Arduino / ESP32 Dependencies -----------------------
#include <Arduino.h> // Library for Arduino-like code

// ---------------------- STD Libraries ------------------------------------
#include <cmath> // Standard library for math (Symbols, constants and oper.)

// ---------------------- Custom dependencies ------------------------------
#include "motor.hpp" // Custom header for a motor class

// /////////////////////// CLASS DEFINITIONS ///////////////////////////////
namespace diff
{
    /**
     * Method to start safely the motors and prevent unexpected movement
     * when initializing board.
     */
    void MotorDriver::safe_init()
    {
        pinMode(this->enable_pin_, OUTPUT);
        analogWrite(this->enable_pin_, 0);

        pinMode(this->forw_pin_, OUTPUT);
        pinMode(this->back_pin_, OUTPUT);

        digitalWrite(this->forw_pin_, LOW);
        digitalWrite(this->back_pin_, LOW);

    } // void MotorDriver::safe_init()

    /**
     * Initialize motor object in a secure way.
     */
    void MotorDriver::begin()
    {
        this->safe_init();
        this->stop();

    } // void MotorDriver::begin()

    /**
     * Set the PWM for the motor velocity, it will clamp it according the max
     * and min PWM allowed. Also, based on the sign, it will determinate
     * the motor direction.
     *
     * @param speed PWM integer value for controlling the motor
     */
    void MotorDriver::set_speed(int speed)
    {
        int abs_speed = abs(speed);

        // Clamp PWM based on absolute value
        if(abs_speed > this->MAX_SPEED)
        {
            abs_speed = this->MAX_SPEED;
        }

        // Write velocity
        analogWrite(this->enable_pin_, abs_speed);

        // Check sign to determinate direction of movement
        if(speed < 0)
        {
            // Move forward
            digitalWrite(this->forw_pin_, HIGH);
            digitalWrite(this->back_pin_, LOW);
        }
        else if(speed > 0)
        {
            // Move backward
            digitalWrite(this->forw_pin_, LOW);
            digitalWrite(this->back_pin_, HIGH);
        }
        else
        {
            // Stop
            digitalWrite(this->forw_pin_, HIGH);
            digitalWrite(this->back_pin_, HIGH);
            analogWrite(this->enable_pin_, 0);
        }

    } // void MotorDriver::set_speed()

    /**
     * Method oriented to hard-force the stop of the motors.
     */
    void MotorDriver::stop()
    {
        analogWrite(this->enable_pin_, 0);
        digitalWrite(this->forw_pin_, LOW);
        digitalWrite(this->back_pin_, LOW);

    } // void MotorDriver::stop()

} // namespace diff
