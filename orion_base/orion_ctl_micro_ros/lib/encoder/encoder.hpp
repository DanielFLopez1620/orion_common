/*
 * @file encoder.hpp
 * @brief Quadrature encoder driver for ORION wheel odometry.
 *
 * Single-channel interrupt-driven encoder reading (ENCA triggers ISR,
 * ENCB sampled inside ISR for direction). Provides atomic tick count
 * and reset for PID feedback loops.
 */

#ifndef ENCODER_HPP
#define ENCODER_HPP

// Required for the IRAM_ATTR attribute used on the ISR handler below.
#include <Arduino.h>

namespace diff
{
    /*
     * Quadrature encoder decoder for motor speed/position feedback.
     * Implements single-channel counting (ENCA edge-triggered, ENCB sampled).
     * Maintains cumulative encoder count (volatile for ISR safety).
     */
    class EncoderDriver
    {
    private:
        int enc_a_{0};        // GPIO pin for encoder channel A (interrupt source)
        int enc_b_{0};        // GPIO pin for encoder channel B (sampled in ISR)
        volatile int pos_i_{0};  // Cumulative encoder count (modified by ISR)

    public:
        /*
         * Constructor that sets up encoder GPIO pins.
         *
         * @param enc_a GPIO pin for channel A (edge-triggered)
         * @param enc_b GPIO pin for channel B (direction indicator)
         */
        EncoderDriver(const int& enc_a, const int& enc_b)
            : enc_a_{enc_a}, enc_b_ {enc_b}
        {}

        /*
         * Initializes encoder pins as inputs.
         * Call once during setup, before attachInterrupt.
         */
        void begin();

        /*
         * Reads current encoder count with interrupt masking (atomic).
         *
         * @return Cumulative encoder ticks (can be negative)
         */
        int read();

        /*
         * ISR handler for encoder channel A (single-channel quadrature).
         * Samples channel B to determine direction: A_diff != B means backward.
         * MUST be attached as ISR callback, not called directly.
         */
        void IRAM_ATTR readEnc();

        /*
         * Resets encoder count to zero.
         */
        void reset();

    }; // class EncoderDriver

} // namespace diff

#endif