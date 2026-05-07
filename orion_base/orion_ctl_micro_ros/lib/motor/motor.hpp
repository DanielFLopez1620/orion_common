#ifndef MOTOR_HPP
#define MOTOR_HPP

namespace diff
{
    /*
     * DC motor driver abstraction for L298N-style motor drivers.
     * Controls speed (PWM) and direction via GPIO pins.
     */
    class MotorDriver
    {
    private:
        unsigned int enable_pin_{0};  // PWM pin for speed control
        unsigned int forw_pin_{0};    // Direction pin (forward control)
        unsigned int back_pin_{0};    // Direction pin (backward control)

    public:
        /*
         * Constructor that sets up GPIO pins for motor control.
         * @param enable GPIO pin for PWM speed control
         * @param forward GPIO pin for forward direction
         * @param backward GPIO pin for backward direction
         */
        MotorDriver(
            const unsigned int& enable,
            const unsigned int& forward,
            const unsigned int& backward)
            :
            enable_pin_{enable},
            forw_pin_{forward},
            back_pin_{backward}
        {}

        /*
         * Initializes pins and brings motor to safe idle state.
         * Call once during setup.
         */
        void begin();

        /*
         * Sets motor speed and direction via PWM.
         * @param speed PWM value: negative=forward, positive=backward, 0=stop
         *        Range: [-255, 255] (0-255 mapped to PWM duty cycle)
         */
        void setSpeed(int speed);

        /*
         * Stops motor immediately by clearing PWM and direction pins.
         */
        void stop();

        /*
         * Safely initializes all motor pins to OUTPUT and LOW state.
         * Prevents accidental motor startup during initialization.
         */
        void safeInit();

    }; // class MotorDriver

} // diff

#endif
