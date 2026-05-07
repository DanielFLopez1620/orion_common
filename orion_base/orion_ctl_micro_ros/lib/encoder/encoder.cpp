// //////////////////////// Include Libraries //////////////////////////////
// -------------------- Arduino / ESP32 Dependencies -----------------------
#include "Arduino.h"   // Library oriented to use Arduino-like definitions

// -------------------- Custom dependencies --------------------------------
#include "encoder.hpp" // Encoder header

// ////////////////////// CLASS DEFINITIONS ////////////////////////////////
namespace diff
{
    void EncoderDriver::begin()
    {
        pinMode(this->enc_a_, INPUT);
        pinMode(this->enc_b_, INPUT);
    }

    int EncoderDriver::read()
    {
        int pos = 0;
        noInterrupts();
        pos = this->pos_i_;
        interrupts();
        return pos;
    }

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
    }

    void EncoderDriver::reset()
    {
        this->pos_i_ = 0;
    }

} // namespace diff