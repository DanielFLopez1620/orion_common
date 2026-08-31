#include "touch_driver.hpp"

#include <Arduino.h>

TouchSensorDriver::TouchSensorDriver(unsigned int pin) : pin(pin) {}

void TouchSensorDriver::initialize()
{
    pinMode(pin, INPUT);
}

bool TouchSensorDriver::read()
{
    bool new_state = digitalRead(pin);
    changed = (new_state != state);
    state = new_state;
    return state;
}

bool TouchSensorDriver::hasChanged() const
{
    return changed;
}

bool TouchSensorDriver::getState() const
{
    return state;
}
