#include "pid.hpp"
#include "constants.hpp"

namespace diff
{
    void ControlPID::compute(int enc_count, int& computed_output)
    {
        if(!this->enabled_)
        {
            if(this->last_input_ != 0.0f)
            {
                reset(enc_count);
            }
            return;
        }

        float input = (float)(enc_count - this->last_enc_count_);
        float err = this->setpoint_ - input;
        float output = (this->kp_ * err - this->kd_ * (input - this->last_input_)
            + this->integral_term_) / this->ko_;
        output += this->last_output_;

        // Dead-zone compensation: only applied when setpoint direction matches output direction.
        // Avoids bang-bang behavior when decelerating or reversing through zero.
        if (this->setpoint_ > 0.0f && output > 0.0f && output < diff::ROBOT_CONST::PWM_DEADZONE) {
            output = diff::ROBOT_CONST::PWM_DEADZONE;
        } else if (this->setpoint_ < 0.0f && output < 0.0f && output > -diff::ROBOT_CONST::PWM_DEADZONE) {
            output = -diff::ROBOT_CONST::PWM_DEADZONE;
        }

        if(output > (float)this->pwm_max_)
        {
            output = (float)this->pwm_max_;
        }
        else if (output < (float)this->pwm_min_)
        {
            output = (float)this->pwm_min_;
        }
        else
        {
            this->integral_term_ += this->ki_ * err;
        }

        computed_output = (int)output;

        this->last_enc_count_ = enc_count;
        this->last_input_ = input;
        this->last_output_ = output;
    }

    void ControlPID::disable()
    {
        this->enabled_ = false;
    }

    void ControlPID::enable()
    {
        this->enabled_ = true;
    }

    bool ControlPID::enabled()
    {
        return this->enabled_;
    }

    void ControlPID::reset(int enc_count)
    {
        this->setpoint_ = 0.0f;
        this->integral_term_ = 0.0f;
        this->last_enc_count_ = enc_count;
        this->last_input_ = 0.0f;
        this->last_output_ = 0.0f;
    }

    void ControlPID::setSetpoint(float setpoint)
    {
        this->setpoint_ = setpoint;
    }

    void ControlPID::setTunings(float kp, float kd, float ki, float ko)
    {
        this->kp_ = kp;
        this->kd_ = kd;
        this->ki_ = ki;
        this->ko_ = ko;
    }

} // namespace diff
