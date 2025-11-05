// //////////////////////// Include Libraries //////////////////////////////
// -------------------- Arduino / ESP32 Dependencies -----------------------
#include "Arduino.h"   // Library oriented to use Arduino-like definitions

// -------------------- Custom dependencies --------------------------------
#include "encoder.hpp" // Encoder header

// ////////////////////// CLASS DEFINITIONS ////////////////////////////////
namespace diff
{
    /**
     * Initialize encoder.
     * Set up pin mode for channel A & B.
     */
    void EncoderDriver::begin()
    {
        pinMode(this->enc_a_, INPUT);
        pinMode(this->enc_b_, INPUT);

    } // void EncoderDriver::begin()

    /**
     * Update encoder count by saving the value of the encoder count sum
     * with a blocking of the interruptions.
     *
     * @return Integer with the encoder count sum
     */
    int EncoderDriver::read()
    {
        int pos = 0;
        noInterrupts();
        pos = this->pos_i_;
        interrupts();
        return pos;

    } // int EncoderDriver::read()

    /**
     * Function that determinate the direction of the movement (positive is
     * going forward, negative otherwise) and update the encoder count.
     */
    void IRAM_ATTR EncoderDriver::readEnc()
    {
        if(digitalRead(this->enc_a_) != digitalRead(this->enc_b_))
        {
            this->pos_i_--;
        }
        else
        {
            this->pos_i_++;
        }

    } // void EncoderDriver::readEnc()

    /**
     * Resets to zero the encoder count
     */
    void EncoderDriver::reset()
    {
        this->pos_i_ = 0;

    } // void EncodeDriver::reset()

} // namespace diff