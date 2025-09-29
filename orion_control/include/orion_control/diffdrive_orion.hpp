#ifndef DIFFDRIVE_ORION_HPP
#define DIFFDRIVE_ORION_HPP

#include <hardware_interface/handle.hpp>
#include <hardware_interface/hardware_info.hpp>
#include <hardware_interface/system_interface.hpp>
#include <hardware_interface/types/hardware_interface_return_values.hpp>
#include <hardware_interface/types/hardware_interface_type_values.hpp>

#include <rclcpp/rclcpp.hpp>

#include "orion_control/wheel.hpp"
#include "orion_control/orion_shared_state.hpp"

namespace orion_control
{
    struct Config
    {
        std::string wheel_left_name;
        std::string wheel_right_name;
        int enc_tics_per_rev;
    };

    class DiffDriveOrion : public hardware_interface::SystemInterface
    {
    public:
        DiffDriveOrion() = default;

        hardware_interface::CallbackReturn on_init(const hardware_interface::HardwareInfo& info) override;

        hardware_interface::CallbackReturn on_configure(const rclcpp_lifecycle::State& prev_state) override;

        std::vector<hardware_interface::StateInterface> export_state_interfaces() override;

        std::vector<hardware_interface::CommandInterface> export_command_interfaces() override;

        hardware_interface::CallbackReturn on_activate(const rclcpp_lifecycle::State& prev_state) override;

        hardware_interface::CallbackReturn on_deactivate(const rclcpp_lifecycle::State& prev_state) override;

        hardware_interface::return_type read(const rclcpp::Time& time, const rclcpp::Duration& period) override;

        hardware_interface::return_type write(const rclcpp::Time&, const rclcpp::Duration& period) override;

    private:
        rclcpp::Logger logger_{rclcpp::get_logger("DiffDriveOrion")};

        orion_control::Config config_;
        Wheel wheel_left_;
        Wheel wheel_right_;
    };

} // namespace orion_control

#endif