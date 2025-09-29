#include "orion_control/forward_orion.hpp"

namespace orion_control
{
    hardware_interface::CallbackReturn ForwardOrion::on_init(const hardware_interface::HardwareInfo& info)
    {
        if (SystemInterface::on_init(info) != hardware_interface::CallbackReturn::SUCCESS)
        {
            return hardware_interface::CallbackReturn::ERROR;
        }

        {
            std::lock_guard<std::mutex> lock(g_orion_forward_allocation_mtx);
            g_orion_forw_state.servos.emplace_back();
            servo_ptr_ = &g_orion_forw_state.servos.back();
        }

        servo_.Setup(info_.hardware_parameters.at("servo_name"));

        RCLCPP_INFO(this->logger_, "Forward Control: Init servo %s", servo_.joint_name_.c_str());

        return hardware_interface::CallbackReturn::SUCCESS;
    }

    std::vector<hardware_interface::StateInterface> ForwardOrion::export_state_interfaces()
    {
        return {
            hardware_interface::StateInterface(
            servo_.joint_name_, hardware_interface::HW_IF_POSITION, &servo_.pos_)
        };
    }

    std::vector<hardware_interface::CommandInterface> ForwardOrion::export_command_interfaces()
    {
        return {
            hardware_interface::StateInterface(
                servo_.joint_name_, hardware_interface::HW_IF_POSITION, &servo_.cmd_)
        };
    }

    hardware_interface::return_type ForwardOrion::read(const rclcpp::Time&, const rclcpp::Duration&)
    {
        servo_.pos_ = servo_ptr_->pos.load();
        return hardware_interface::return_type::OK;
    }

    hardware_interface::return_type ForwardOrion::write(const rclcpp::Time&, const rclcpp::Duration&)
    {
        servo_ptr_->cmd.store(servo_.cmd_);
        return hardware_interface::return_type::OK;
    }
}

#include <pluginlib/class_list_macros.hpp>
PLUGINLIB_EXPORT_CLASS(orion_control::ForwardOrion, hardware_interface::SystemInterface)
