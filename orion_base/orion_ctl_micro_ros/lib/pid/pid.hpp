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
        float kp_{0.0f};             // Proportional constant
        float kd_{0.0f};             // Derivative constant
        float ki_{0.0f};             // Integrative constant
        float ko_{0.0f};             // Gain constant
        int pwm_max_{0};             // Max PWM
        int pwm_min_{0};             // Min PWM
        bool enabled_{false};        // PID state
        float setpoint_{0.0f};       // Set Point
        float integral_term_{0.0f};  // Integral cumulative sum
        long last_enc_count_{0};     // Last encoder value
        float last_input_{0.0f};     // Last input
        float last_output_{0.0f};    // Last output
    public:

        /**
         * User defined constructor that set up the constants and the PWM
         * limits.
         *
         * @param kp Proportional constant
         * @param kd Derivative constant
         * @param ki Integral constant
         * @param ko Gain constant
         * @param pwm_max Max PWM output
         * @param pwm_min Min PWM output
         */
        ControlPID(float kp, float kd, float ki, float ko,
            int pwm_max, int pwm_min)
            : kp_{kp}, kd_{kd}, ki_{ki}, ko_{ko}, pwm_min_{pwm_min}, pwm_max_{pwm_max}
            {}

        // Method prototypes

        void compute(int enc_count, int& computed_output);

        void disable();

        void enable();

        bool enabled();

        void reset(int enc_count);

        void setSetpoint(float setpoint);

        void setTunings(float kp, float kd, float ki, float ko);

    }; // class ControlPID

} // namespace diff

#endif