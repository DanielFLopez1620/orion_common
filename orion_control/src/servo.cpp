#include <cmath>
#include "orion_control/servo.hpp"

namespace orion_control
{
    /**
     * Setting servo by providing joint name
     * 
     * @param joint_name Name of the revolute joint on URDF
     */
    void Servo::Setup(const std::string& joint_name)
    {
        this->joint_name_ = joint_name;
    }

} // orion_control