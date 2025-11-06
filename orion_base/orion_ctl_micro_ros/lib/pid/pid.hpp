#ifndef PID_HPP
#define PID_HPP

namespace diff
{
    /**
     * Class oriented to implement a PID controller which input is the encoder
     * count and the output is the PWM required to move the motors that way.
     */
    class ControlPID
    {
    private:
        int kp_{0};               // Proportional constant
        int kd_{0};               // Derivative constant
        int ki_{0};               // Integrative constant
        int ko_{0};               // Gain constant
        int pwm_max_{0};          // Max PWM
        int pwm_min_{0};          // Min PWM
        bool enabled_{false};     // PID state
        int setpoint_{0};         // Set Point
        int integral_term_{0};    // Integral cumulative sum
        long last_enc_count_{0};  // Last encoder value
        int last_input_{0};       // Last input
        long last_output_{0};     // Last output
    public:

        /**
         * User defined constructor that set up the constants and the PWM
         * limits.
         *
         * @param kp Proportional constant
         * @param kd Derivative constant
         * @param ki Integral constant
         * @param ko Gain constant
         * @param pwm_min Max PWM output
         * @param pwm_max Min PWM output
         */
        ControlPID(int kp, int kd, int ki, int ko,
            int pwm_max, int pwm_min)
            : kp_{kp}, ki_{ki}, kd_ {kd}, ko_{ko}, pwm_min_{pwm_min}, pwm_max_(pwm_max)
            {}

        // Method prototypes

        void compute(int enc_count, int& computed_output);

        void disable();

        void enable();

        bool enabled();

        void reset(int enc_count);

        void setSetpoint(int setpoint);

        void setTunings(int kp, int kd, int ki, int ko);

    }; // class ControlPID

} // namespace diff

#endif