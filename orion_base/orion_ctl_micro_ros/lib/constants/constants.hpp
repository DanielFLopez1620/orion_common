#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

namespace diff
{
    /* Motor control constants: PID tuning, PWM limits, and timing.
       Used by ControlPID and motor driver classes. */
    struct ROBOT_CONST
    {
        // =============== PID Control Loop ===============

        // PID control rate (Hz) — 20 Hz = 50 ms period
        static const int PID_RATE {20};

        // PID period in milliseconds (derived from PID_RATE)
        static const int PID_T {1000 / PID_RATE};

        // Proportional gain (KP ~ 35.0: responds quickly to error)
        static constexpr float PID_KP {35.0f};

        // Derivative gain (KD ~ 8.0: dampens oscillations)
        static constexpr float PID_KD {8.0f};

        // Integral gain (KI ~ 0.05: removes steady-state error)
        static constexpr float PID_KI {0.05f};

        // Output gain divisor (KO ~ 50.0: scales output to PWM range)
        static constexpr float PID_KO {50.0f};

        // =============== PWM Motor Control ===============

        // Maximum PWM output (0-255), clamping limit
        static const int PWM_MAX {255};

        // Minimum PWM output (-255), clamping limit
        static const int PWM_MIN {-255};

        /* Dead-zone compensation: minimum PWM to overcome static friction.
           If computed PWM is between -DZ and +DZ, clamp to ±DZ to ensure motion. */
        static constexpr float PWM_DEADZONE {50.0f};
    };
}

#endif
