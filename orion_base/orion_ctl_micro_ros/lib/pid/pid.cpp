// //////////////////////// Include Libraries //////////////////////////////
// ------------------------- Custom dependencies ---------------------------
#include "pid.hpp" // Custom PID class header
#include "constants.hpp" // Robot constants (PID_RATE, gains, PWM limits)

// ////////////////////////// CLASS DEFINITIONS ////////////////////////////
namespace diff
{
    void ControlPID::compute(int enc_count, int& computed_output)
    {
        // Check that the PID is disabled
        if(!this->enabled_)
        {
            // If so, take advantage to reset the PID.
            if(this->last_input_ != 0.0f)
            {
                reset(enc_count);
            }
            return;
        }

        // Set up input (encoder ticks since last call)
        float input = (float)(enc_count - this->last_enc_count_);

        // Determinate the error
        float err = this->setpoint_ - input;

        // Calculate output
        // Considering PID with a Ko value to avoid too much increment
        float output = (this->kp_ * err - this->kd_ * (input - this->last_input_)
            + this->integral_term_) / this->ko_;

        // Sum previous output
        output += this->last_output_;

        // Dead-zone compensation: only applied when setpoint direction matches output direction.
        // Avoids bang-bang behavior when decelerating or reversing through zero.
        if (this->setpoint_ > 0.0f && output > 0.0f && output < diff::ROBOT_CONST::PWM_DEADZONE) {
            output = diff::ROBOT_CONST::PWM_DEADZONE;
        } else if (this->setpoint_ < 0.0f && output < 0.0f && output > -diff::ROBOT_CONST::PWM_DEADZONE) {
            output = -diff::ROBOT_CONST::PWM_DEADZONE;
        }

        // Clamp to avoid saturation
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
            // If there is no saturation, add the integral term for next iter.
            this->integral_term_ += this->ki_ * err;
        }

        // Update value of the commanded output
        computed_output = (int)output;

        // Prepare values for the next iteration
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
