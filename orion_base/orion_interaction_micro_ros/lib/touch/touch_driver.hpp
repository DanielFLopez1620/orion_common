/*
 * @file touch_driver.hpp
 * @brief Capacitive touch sensor abstraction for ORION interaction ESP32.
 *
 * Wraps a single digital touch sensor pin, providing debounced state
 * reads. Each of the four ORION touch pads (UR, UL, LR, LL) is
 * represented by one instance of this class.
 */

#ifndef TOUCH_DRIVER_HPP
#define TOUCH_DRIVER_HPP

/*
 * Single capacitive touch sensor driver (digital GPIO input).
 *
 * Responsibilities:
 * - Configure the pin as INPUT
 * - Read the current state
 * - Detect state changes since the last read (simple debounce)
 */
class TouchSensorDriver
{
public:
    /*
     * @param pin GPIO pin connected to the touch sensor output.
     */
    explicit TouchSensorDriver(unsigned int pin);

    /*
     * Configures the pin as INPUT. Must be called once during setup().
     */
    void initialize();

    /*
     * Reads the current sensor state and updates internal state.
     *
     * @return true if touched, false otherwise.
     */
    bool read();

    /*
     * @return true if the state changed on the last call to read().
     */
    bool hasChanged() const;

    /*
     * @return last state captured by read(), without re-sampling hardware.
     */
    bool getState() const;

private:
    unsigned int pin;
    bool state = false;
    bool changed = false;
};

#endif
