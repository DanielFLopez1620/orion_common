/*
 * @file pid.hpp
 * @brief Discrete PID controller for differential drive motor speed regulation.
 *
 * Computes PWM output from encoder feedback, with configurable gains, output
 * clamping, and a dead-zone guard to prevent motor stall at low commands.
 */

#ifndef PID_HPP
#define PID_HPP

namespace diff
{
    /*
     * PID controller for motor speed feedback loop.
     * Takes encoder count as input, outputs PWM command to motor.
     * Features: dead-zone compensation, integral clamping, anti-windup.
     */
    class ControlPID
    {
    private:
        float kp_{0.0f};             // Proportional gain
        float kd_{0.0f};             // Derivative gain
        float ki_{0.0f};             // Integral gain
        float ko_{0.0f};             // Output gain (saturation divisor)
        int pwm_max_{0};             // Max PWM output (0-255)
        int pwm_min_{0};             // Min PWM output (0-255)
        bool enabled_{false};        // Controller state
        float setpoint_{0.0f};       // Target encoder delta per cycle
        float integral_term_{0.0f};  // Cumulative integral error
        long last_enc_count_{0};     // Previous encoder reading
        float last_input_{0.0f};     // Previous encoder delta
        float last_output_{0.0f};    // Previous PWM output
    public:

        /*
         * Constructor that sets PID gains and PWM limits.
         *
         * @param kp Proportional gain
         * @param kd Derivative gain
         * @param ki Integral gain
         * @param ko Output gain divisor
         * @param pwm_max Maximum PWM output (0-255)
         * @param pwm_min Minimum PWM output (0-255)
         */
        ControlPID(float kp, float kd, float ki, float ko,
            int pwm_max, int pwm_min)
            : kp_{kp}, kd_{kd}, ki_{ki}, ko_{ko}, pwm_min_{pwm_min}, pwm_max_{pwm_max}
            {}

        /*
         * Computes PID control output based on encoder feedback.
         * Dead-zone compensation and clamping applied automatically.
         * Only computes if controller is enabled.
         *
         * @param enc_count Current encoder count from motor
         * @param computed_output Reference to PWM output [-255, 255]
         */
        void compute(int enc_count, int& computed_output);

        /*
         * Disables PID computation. Next call to compute() will reset integrator.
         */
        void disable();

        /*
         * Enables PID computation.
         */
        void enable();

        /*
         * Checks if PID controller is currently enabled.
         *
         * @return True if enabled, false otherwise
         */
        bool enabled();

        /*
         * Resets integrator and state variables to initial condition.
         *
         * @param enc_count Current encoder reading (used as baseline)
         */
        void reset(int enc_count);

        /*
         * Sets target encoder delta per control cycle.
         *
         * @param setpoint Target encoder count change (ticks/cycle)
         */
        void setSetpoint(float setpoint);

        /*
         * Updates PID tuning gains at runtime.
         *
         * @param kp Proportional gain
         * @param kd Derivative gain
         * @param ki Integral gain
         * @param ko Output gain divisor
         */
        void setTunings(float kp, float kd, float ki, float ko);

    }; // class ControlPID

} // namespace diff

#endif