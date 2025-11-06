#ifndef HARDWARE_HPP
#define HARDWARE_HPP

namespace diff
{
    struct HARDWARE
    {
        // ------------------- Motor Left --------------------------

        // NOTE: If the feedback received doesn't match the dir,
	    // then proceed to exchange the encoder ports.

        // Encoder Channel A
        static const unsigned int ML_ENCA = 33;
        // Encoder Channel B
        static const unsigned int ML_ENCB = 32;

	    // NOTE: If the direction of the motors is reversed,
	    // then proceed to exchange Forward and Backward ports.

        // Driver Forward Pin
        static const unsigned int ML_FORW = 21;
        // Driver Backward Pin
        static const unsigned int ML_BACW = 22;
        // Driver Enable Pin
        static const unsigned int ML_EN = 17;

        // ------------------- Motor RIGHT --------------------------

        // NOTE: If the feedback received doesn't match the dir,
        // then proceed to exchange the encoder ports.

        // Encoder Channel A
        static const unsigned int MR_ENCA = 34;
        // Encoder Channel B
        static const unsigned int MR_ENCB = 35;
        // Driver Forward Pin

	    // NOTE: If the direction of the motors is reversed,
        // then proceed to exchange Forward and Backward ports.

        static const unsigned int MR_FORW = 18;
        // Driver Backward Pin
        static const unsigned int MR_BACW = 19;
        // Driver Enable Pin
        static const unsigned int MR_EN = 16;

    }; // struct HARDWARE

} // diff

namespace fwd
{
    struct HARDWARE
    {
        // Servo left
        static const unsigned int SERVO_LEFT = 25;

        // Servo right
        static const unsigned int SERVO_RIGHT = 23;

    }; // struct HARDWARE

}  // fwd

#endif
