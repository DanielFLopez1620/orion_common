#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

namespace diff
{
    /**
     * Struct that contains the constants of the controller. This aims for a
     * PID controller that implements clamping and anti-saturation mechanism
     *
     * Attributes list:
     * - PID_RATE : Hz rate of the PID
     * - PID_T : Period of the PID.
     * - PID_KP : Proportional constant
     * - PID_KD : Derivative constant
     * - PID_KI : Integral constant
     * - PID_KO : Constant to avoid big number at control output
     * - PWM_MAX : Max pwm. If output is greater, clamp.
     * - PWM_MIN : Min pwm. If output is less, clamp.
     * - PWM_DEADZONE : Min PWM threshold to overcome motor static friction (~50-70)
     */
    struct ROBOT_CONST
    {
        static const int PID_RATE {20};
        static const int PID_T {1000 / PID_RATE};
        static constexpr float PID_KP {35.0f};
        static constexpr float PID_KD {8.0f};
        static constexpr float PID_KI {0.05f};
        static constexpr float PID_KO {50.0f};
        static const int PWM_MAX {255};
        static const int PWM_MIN {-255};
        static constexpr float PWM_DEADZONE {50.0f};
    };
}

#endif
