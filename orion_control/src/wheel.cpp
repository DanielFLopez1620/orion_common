#include <cmath>
#include "orion_control/wheel.hpp"

namespace orion_control
{
    /**
     * Set up the wheel by providing the joint name and the value of rads moved
     * as a new tick is detected.
     *
     * @param wheel_name Name of the continuous joint on URDF
     * @param tickers_per_rev
     */
    void Wheel::Setup(const std::string& wheel_name, int tics_per_rev)
    {
        this->name_ = wheel_name;
        this->rads_per_tick_ = (2 * M_PI) / tics_per_rev;
    }

    /**
     * Getter of the current angle of the wheel
     *
     * @return Current position in radians.
     */
    double Wheel::Angle()
    {
        return this->enc_ * this->rads_per_tick_;
    }

} // orion_control
