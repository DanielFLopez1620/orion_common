#include <cmath>
#include "orion_control/wheel.hpp"

namespace orion_control
{
    void Wheel::Setup(const std::string& wheel_name, int tics_per_rev)
    {
        this->name_ = wheel_name;
        this->rads_per_tick_ = (2 * M_PI) / tics_per_rev;
    }

    double Wheel::Angle()
    {
        return this->enc_ * this->rads_per_tick_;
    }

} // orion_control
