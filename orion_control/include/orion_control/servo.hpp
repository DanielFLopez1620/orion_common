#ifndef SERVO_HPP
#define SERVO_HPP

#include <string>

namespace orion_control
{
    class Servo
    {
    public:
        Servo() = default;

        void Setup(const std::string& joint_name);

        std::string joint_name_;   // Joint name
        double pos_{0};            // Current position
        double cmd_{0};            // Current commanded position

    }; // class Servo
} // orion_control

#endif