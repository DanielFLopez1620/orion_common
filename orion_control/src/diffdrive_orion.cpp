#include "orion_control/diffdrive_orion.hpp"

namespace orion_control
{
    hardware_interface::CallbackReturn DiffDriveOrion::on_init(const hardware_interface::HardwareInfo& info)
    {
        if(hardware_interface::SystemInterface::on_init(info) != hardware_interface::CallbackReturn::SUCCESS)
        {
            return hardware_interface::CallbackReturn::ERROR;
        }

        RCLCPP_INFO(this->logger_, "DiffDriveOrion: on_init...");

        config_.wheel_left_name = info_.hardware_parameters.at("left_wheel");
        config_.wheel_right_name = info_.hardware_parameters.at("right_wheel");
        config_.enc_tics_per_rev = std::stoi(info_.hardware_parameters.at("enc_tics_per_rev"));

        wheel_left_.Setup(config_.wheel_left_name, config_.enc_tics_per_rev);
        wheel_right_.Setup(config_.wheel_right_name, config_.enc_tics_per_rev);

        return hardware_interface::CallbackReturn::SUCCESS;
    }

    hardware_interface::CallbackReturn DiffDriveOrion::on_configure(const rclcpp_lifecycle::State &)
    {
        RCLCPP_INFO(this->logger_, "DiffDriveOrion: on_configure...");
        return hardware_interface::CallbackReturn::SUCCESS;
    }

    std::vector<hardware_interface::StateInterface> DiffDriveOrion::export_state_interfaces()
    {
        std::vector<hardware_interface::StateInterface> state_interfaces;

        state_interfaces.emplace_back(
            hardware_interface::StateInterface(wheel_left_.name_,
                hardware_interface::HW_IF_VELOCITY, &wheel_left_.vel_));
        state_interfaces.emplace_back(
            hardware_interface::StateInterface(wheel_left_.name_,
                hardware_interface::HW_IF_POSITION, &wheel_left_.pos_));
        state_interfaces.emplace_back(
            hardware_interface::StateInterface(wheel_right_.name_,
                hardware_interface::HW_IF_VELOCITY, &wheel_right_.vel_));
        state_interfaces.emplace_back(
            hardware_interface::StateInterface(wheel_right_.name_,
                hardware_interface::HW_IF_POSITION, &wheel_right_.pos_));

        return state_interfaces;
    }

    std::vector<hardware_interface::CommandInterface> DiffDriveOrion::export_command_interfaces()
    {
        std::vector<hardware_interface::CommandInterface> command_interfaces;

        command_interfaces.emplace_back(
            hardware_interface::CommandInterface(wheel_left_.name_,
                hardware_interface::HW_IF_VELOCITY, &wheel_left_.cmd_));
        command_interfaces.emplace_back(
            hardware_interface::CommandInterface(wheel_right_.name_,
                hardware_interface::HW_IF_VELOCITY, &wheel_right_.cmd_));

        return command_interfaces;
    }

    hardware_interface::CallbackReturn DiffDriveOrion::on_activate(const rclcpp_lifecycle::State &)
    {
        RCLCPP_INFO(this->logger_, "DiffDriveOrion: on_activate...");
        return hardware_interface::CallbackReturn::SUCCESS;
    }

    hardware_interface::CallbackReturn DiffDriveOrion::on_deactivate(const rclcpp_lifecycle::State &)
    {
        RCLCPP_INFO(this->logger_, "DiffDriveOrion: on_deactivate...");
        return hardware_interface::CallbackReturn::SUCCESS;
    }

    hardware_interface::return_type DiffDriveOrion::read(const rclcpp::Time &, const rclcpp::Duration & period)
    {
        const double dt = period.seconds();

        wheel_left_.enc_ = g_orion_diff_state.enc_left.load(std::memory_order_relaxed);
        wheel_right_.enc_ = g_orion_diff_state.enc_right.load(std::memory_order_relaxed);

        const double left_prev = wheel_left_.pos_;
        wheel_left_.pos_ = wheel_left_.Angle();
        wheel_left_.vel_ = (wheel_left_.pos_ - left_prev) / dt;

        const double right_prev = wheel_right_.pos_;
        wheel_right_.pos_ = wheel_right_.Angle();
        wheel_right_.vel_ = (wheel_right_.pos_ - right_prev) / dt;

        return hardware_interface::return_type::OK;
    }

    hardware_interface::return_type DiffDriveOrion::write(const rclcpp::Time &, const rclcpp::Duration &)
    {
        const int left_cmd = static_cast<int>(wheel_left_.cmd_ / wheel_left_.rads_per_tick_);
        const int right_cmd = static_cast<int>(wheel_right_.cmd_ / wheel_right_. rads_per_tick_);

        g_orion_diff_state.cmd_left.store(left_cmd, std::memory_order_relaxed);
        g_orion_diff_state.cmd_right.store(right_cmd, std::memory_order_relaxed);

        return hardware_interface::return_type::OK;
    }
}

#include <pluginlib/class_list_macros.hpp>
PLUGINLIB_EXPORT_CLASS(orion_control::DiffDriveOrion, hardware_interface::SystemInterface)