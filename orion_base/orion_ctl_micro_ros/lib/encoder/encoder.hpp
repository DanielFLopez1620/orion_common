#ifndef ENCODER_HPP
#define ENCODER_HPP

namespace diff
{
    /**
     * Class oriented to define two-channel encoders to get feedback of the
     * direction and speed of a DC Motor.
     */
    class EncoderDriver
    {
    private:
        int enc_a_{0};  // Pin of the encoder's channel A
        int enc_b_{0};  // Pin of the encoder's channel B

        volatile int pos_i_{0};  // Encoder count sum

    public:
        /**
         * User defined constructor to set up the encoder channels.
         *
         * @param enc_a Pin of the encoder's channel A
         * @param enc_b Pin of the encoder's channel B
         */
        EncoderDriver(const int& enc_a, const int& enc_b)
            : enc_a_{enc_a}, enc_b_ {enc_b}
        {}

        void begin();
        int read();
        void IRAM_ATTR readEnc();
        void reset();

    }; // class EncoderDriver

} // namespace diff

#endif