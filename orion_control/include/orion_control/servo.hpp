/*
 * @file servo.hpp
 * @brief Servo state model for the ORION forward-command hardware interface.
 *
 * Stores joint name, current position feedback, and commanded position for a
 * single servo, used by the ForwardOrion hardware interface plugin.
 */

#ifndef SERVO_HPP
#define SERVO_HPP

#include <string>

namespace orion_control
{
    /*
     * Represents a single servo's state for use with
     * the ros2_control ForwardCommand hardware interface.
     */
    class Servo
    {
    public:
        Servo() = default;

        /*
         * Initializes the servo with its URDF joint name.
         *
         * @param joint_name Name of the revolute joint in the URDF.
         */
        void Setup(const std::string& joint_name);

        std::string joint_name_;   // Joint name
        double pos_{0};            // Current position
        double cmd_{0};            // Current commanded position

    }; // class Servo
} // orion_control

#endif
