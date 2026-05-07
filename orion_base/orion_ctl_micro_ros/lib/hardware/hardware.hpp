/*
 * @file hardware.hpp
 * @brief GPIO pin definitions for ORION motor and encoder connections.
 *
 * Defines pinout for:
 * - Motor drivers (forward, backward, enable)
 * - Encoders (channel A, B for quadrature decoding)
 * - Servo motors (PWM control pins)
 *
 * If encoder feedback or motor direction doesn't match hardware,
 * exchange the corresponding pins in the constants below.
 */

#ifndef HARDWARE_HPP
#define HARDWARE_HPP

namespace diff
{
    struct HARDWARE
    {
        // =============== Motor Left ===============

        // NOTE: If encoder feedback doesn't match direction,
        // exchange ML_ENCA and ML_ENCB.
        static const unsigned int ML_ENCA = 33;  // Encoder Channel A
        static const unsigned int ML_ENCB = 32;  // Encoder Channel B

        // NOTE: If motor direction is reversed,
        // exchange ML_FORW and ML_BACW.
        static const unsigned int ML_FORW = 21;  // Driver Forward Pin
        static const unsigned int ML_BACW = 22;  // Driver Backward Pin
        static const unsigned int ML_EN = 17;    // Driver Enable (PWM)

        // =============== Motor Right ===============

        // NOTE: If encoder feedback doesn't match direction,
        // exchange MR_ENCA and MR_ENCB.
        static const unsigned int MR_ENCA = 34;  // Encoder Channel A
        static const unsigned int MR_ENCB = 35;  // Encoder Channel B

        // NOTE: If motor direction is reversed,
        // exchange MR_FORW and MR_BACW.
        static const unsigned int MR_FORW = 18;  // Driver Forward Pin
        static const unsigned int MR_BACW = 19;  // Driver Backward Pin
        static const unsigned int MR_EN = 16;    // Driver Enable (PWM)

    }; // struct HARDWARE

} // diff

namespace fwd
{
    struct HARDWARE
    {
        // =============== Servo Motors ===============

        static const unsigned int SERVO_LEFT = 25;   // Left arm servo PWM
        static const unsigned int SERVO_RIGHT = 23;  // Right arm servo PWM

    }; // struct HARDWARE

}  // fwd

#endif
