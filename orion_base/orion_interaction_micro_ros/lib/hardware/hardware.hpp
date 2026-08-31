/*
 * @file hardware.hpp
 * @brief GPIO pin definitions for ORION interaction ESP32 connections.
 *
 * Pinout considering ESP32 Dev-Kit V1 (30 pins)
 *
 * Defines pinout for:
 * - Capacitive touch sensors (upper/lower, left/right)
 *
 * Screen pins (TFT ILI9225) are defined separately in lib/screen/screen.hpp
 * since they are internal details of the Screen driver.
 *
 * If a touch sensor reads inverted or doesn't match its physical position,
 * check the wiring against the pins defined below.
 */

#ifndef HARDWARE_HPP
#define HARDWARE_HPP

namespace interaction
{
    struct HARDWARE
    {
        // =============== Capacitive Touch Sensors ===============

        static const unsigned int TS_UR_PIN = 4;   // Upper right
        static const unsigned int TS_UL_PIN = 34;  // Upper left
        static const unsigned int TS_LR_PIN = 2;   // Lower right
        static const unsigned int TS_LL_PIN = 35;  // Lower left

    }; // struct HARDWARE

} // namespace interaction

#endif
