#ifndef MOTOR_HPP
#define MOTOR_HPP

namespace diff
{
    /**
     * Class oriented to the control and movement of a DC motor based on
     * the forward, backward and PWM pins of the selected driver.
     */
    class MotorDriver
    {
    private:
        unsigned int enable_pin_{0};  // Port related with speed
        unsigned int forw_pin_{0};    // Forward pin
        unsigned int back_pin_{0};    // Backward pin

    public:
        /**
         * User defined constructor that set up the pins of the driver where
         * the motor is connected.
         *
         * @param enable PWM Pin
         * @param forward Forward Pin
         * @param backward Backward Pin
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

        // Method prototypes
        void begin();
        void set_speed(int speed);
        void stop();
        void safe_init();

    }; // class MotorDriver

} // diff

#endif
