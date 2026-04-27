// //////////////////////// Include Libraries /////////////////////////////
// ---------------------- STD Libraries ------------------------------------
#include <cmath> // Standard library for math (Symbols, constants and oper.)

// ---------------------- Custom dependencies -----------------------------
#include "servo.hpp"

// //////////////////////// CLASS DEFINITIONS /////////////////////////////
namespace fwd
{
    /**
     * Initialize servo by attaching the servo PIN.
     */
    void ServoMotor::begin()
    {
        this->servo_.attach(this->pwm_pin_);
    }

    /**
     * Write the given position (in degrees) to the servo
     *
     * @param degrees Pointer to the desired objective
     */
    void ServoMotor::setPositionDeg(const float& degrees)
    {
        if(degrees > this->max_pos_)
        {
            this->servo_.write(this->max_pos_);
        }
        else if (degrees < this->min_pos_)
        {
            this->servo_.write(this->min_pos_);
        }
        else
        {
            this->servo_.write(degrees);
        }
    }

    /**
     * Read the current position of the servo.
     *
     * @return Current position in degrees.
     */
    float ServoMotor::getPositionDeg()
    {
        return this->servo_.read();
    }

    /**
     * Set the position of the servo by considering radians and then
     * converting them into degrees.
     *
     * @param radians Objective in radians
     */
    void ServoMotor::setPositionRad(const float& radians)
    {
        this->setPositionDeg((radians / M_PI * 180.0));
    }

    /**
     * Read the current position of the servo
     *
     * @return Current position in radians.
     */
    float ServoMotor::getPositionRad()
    {
        return ( (float) this->getPositionDeg() * M_PI / 180.0);
    }

    /**
     * Write the servo position in constant iteration until it reach
     * a defined objective
     */
    void ServoMotor::approximatePositionDeg()
    {
        const int pos = this->getPositionDeg();
        const int diff =  pos - this->objective_;
        if(!(abs(diff) <= 1.0))
        {
            if(diff > 0)
            {
                this->setPositionDeg(pos - (diff > 10 ? 10 : diff));
            }
            else
            {
                this->setPositionDeg(pos + (-diff > 10 ? 10 : -diff));
            }
        }
    }

    /**
     * Set the servo objective in degrees
     *
     * @param degrees Objective to move slowly.
     */
    void ServoMotor::setObjectiveDeg(float degrees)
    {
        this->objective_ = degrees;
    }

    /**
     * Set the servo objective in radians
     *
     * @param degrees Objective to move slowly.
     */
    void ServoMotor::setObjectiveRad(float radians)
    {
        this->setObjectiveDeg(radians * 180 / M_PI);
    }
} // namespace fwd