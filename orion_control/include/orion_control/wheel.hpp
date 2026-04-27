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

        std::string name_ = "";      // Name of the joint
        int enc_ = 0;                // Encoder count
        double cmd_ = 0;             // Current command
        double pos_ = 0;             // Current pose
        double vel_ = 0;             // Current velocity
        double rads_per_tick_ = 0;   // How many radians between tics

    }; // class Wheel

} // orion_control

#endif