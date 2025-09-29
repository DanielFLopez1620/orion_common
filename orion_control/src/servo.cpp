#include <cmath>
#include "orion_control/servo.hpp"

namespace orion_control
{
    void Servo::Setup(const std::string& joint_name)
    {
        this->joint_name_ = joint_name;
    }
}