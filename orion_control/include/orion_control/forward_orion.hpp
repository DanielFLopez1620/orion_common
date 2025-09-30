#ifndef FORWARD_ORION_HPP
#define FORWARD_ORION_HPP

#include <mutex>

#include <hardware_interface/handle.hpp>
#include <hardware_interface/hardware_info.hpp>
#include <hardware_interface/system_interface.hpp>
#include <hardware_interface/types/hardware_interface_return_values.hpp>
#include <hardware_interface/types/hardware_interface_type_values.hpp>

#include <rclcpp/rclcpp.hpp>
#include <rclcpp_lifecycle/state.hpp>

#include <orion_control/servo.hpp>
#include "orion_control/orion_shared_state.hpp"

namespace orion_control
{
    class ForwardOrion : public hardware_interface::SystemInterface
    {
    public:
        ForwardOrion() = default;

        hardware_interface::CallbackReturn on_init(const hardware_interface::HardwareComponentInterfaceParams& params) override;

        hardware_interface::CallbackReturn on_configure(const rclcpp_lifecycle::State& prev_state) override;

        std::vector<hardware_interface::StateInterface> export_state_interfaces() override;

        std::vector<hardware_interface::CommandInterface> export_command_interfaces() override;

        hardware_interface::CallbackReturn on_activate(const rclcpp_lifecycle::State& prev_state) override;

        hardware_interface::CallbackReturn on_deactivate(const rclcpp_lifecycle::State& prev_state) override;

        hardware_interface::return_type read(const rclcpp::Time& time, const rclcpp::Duration& period) override;

        hardware_interface::return_type write(const rclcpp::Time&, const rclcpp::Duration& period) override;

    private:

        ServoState *servo_ptr_ {nullptr};
        Servo servo_;
        rclcpp::Logger logger_{rclcpp::get_logger("ForwardOrion")};
    };
}

#endif