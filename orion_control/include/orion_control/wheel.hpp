/*
 * @file wheel.hpp
 * @brief Wheel state model for the ORION differential drive hardware interface.
 *
 * Stores encoder counts, position, velocity, and command for a single wheel,
 * and provides helpers to convert between encoder ticks and radians.
 */

#ifndef WHEEL_HPP
#define WHEEL_HPP

#include <string>

namespace orion_control
{
    /*
     * Represents a single drive wheel's kinematic state for use with
     * the ros2_control DiffDrive hardware interface.
     */
    class Wheel
    {
    public:
        Wheel() = default;

        /*
         * Initializes the wheel with its URDF joint name and encoder resolution.
         * Computes rads_per_tick_ = 2π / ticks_per_rev.
         * @param joint_name Name of the continuous joint in the URDF.
         * @param ticks_per_rev Encoder pulses per full wheel revolution.
         */
        void Setup(const std::string& joint_name, int ticks_per_rev);

        /*
         * Returns the current wheel angle derived from the encoder count.
         * @return Cumulative wheel angle in radians (enc_ * rads_per_tick_).
         */
        double Angle();

        std::string name_ = "";      // Name of the joint
        int enc_ = 0;                // Encoder count
        double cmd_ = 0;             // Current command
        double pos_ = 0;             // Current pose
        double vel_ = 0;             // Current velocity
        double rads_per_tick_ = 0;   // How many radians between ticks

    }; // class Wheel

} // orion_control

#endif
