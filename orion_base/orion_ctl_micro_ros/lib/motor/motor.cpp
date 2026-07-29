#include <Arduino.h>
#include <cmath>

#include "motor.hpp"

namespace diff
{
    void MotorDriver::safeInit()
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
        this->safeInit();
        this->stop();
    }

    void MotorDriver::setSpeed(int speed)
    {
        int abs_speed = abs(speed);

        analogWrite(this->enable_pin_, abs_speed);

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
