#ifndef WHEEL_HPP
#define WHEEL_HPP

#include <string>

namespace orion_control
{
    class Wheel
    {
    public:
        Wheel() = default;

        void Setup(const std::string& joint_name, int ticks_per_rev);

        double Angle();
        
        std::string name_ = "";
        
        int enc_ = 0;

        double cmd_ = 0;

        double pos_ = 0;

        double vel_ = 0;

        double vel_setpoint_ = 0;
        
        double rads_per_tick_ = 0;
    };
}

#endif