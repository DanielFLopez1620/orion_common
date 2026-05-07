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
    void MotorDriver::safe_init()
    {
        pinMode(this->enable_pin_, OUTPUT);
        analogWrite(this->enable_pin_, 0);

        pinMode(this->forw_pin_, OUTPUT);
        pinMode(this->back_pin_, OUTPUT);

        digitalWrite(this->forw_pin_, LOW);
        digitalWrite(this->back_pin_, LOW);

    }

    void MotorDriver::begin()
    {
        this->safe_init();
        this->stop();

    }

    void MotorDriver::set_speed(int speed)
    {
        int abs_speed = abs(speed);

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

    }

    void MotorDriver::stop()
    {
        analogWrite(this->enable_pin_, 0);
        digitalWrite(this->forw_pin_, LOW);
        digitalWrite(this->back_pin_, LOW);

    }

} // namespace diff
