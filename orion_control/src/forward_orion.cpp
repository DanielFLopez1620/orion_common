#include "orion_control/forward_orion.hpp"

namespace orion_control
{
    hardware_interface::CallbackReturn ForwardOrion::on_init(const hardware_interface::HardwareComponentInterfaceParams& params)
    {
        if (SystemInterface::on_init(params) != hardware_interface::CallbackReturn::SUCCESS)
        {
            return hardware_interface::CallbackReturn::ERROR;
        }
        auto info_ = params.hardware_info;

        servo_.Setup(info_.hardware_parameters.at("servo_name"));

        if(servo_.joint_name_.find("left") != std::string::npos)
        {
            servo_ptr_ = &g_orion_forw_state.servos[0];
        }
        else if(servo_.joint_name_.find("right") != std::string::npos)
        {
            servo_ptr_ = &g_orion_forw_state.servos[1];
        }
        else
        {
            return hardware_interface::CallbackReturn::ERROR;
        }

        // RCLCPP_INFO(this->logger_, "Forward Control: Init servo {}", servo_.joint_name_);

        return hardware_interface::CallbackReturn::SUCCESS;
    }

    std::vector<hardware_interface::StateInterface> ForwardOrion::export_state_interfaces()
    {
        std::vector<hardware_interface::StateInterface> state_interfaces;
        state_interfaces.emplace_back(
            servo_.joint_name_,
            hardware_interface::HW_IF_POSITION,
            &servo_.cmd_
        );
        return state_interfaces;
    }

    std::vector<hardware_interface::CommandInterface> ForwardOrion::export_command_interfaces()
    {
        std::vector<hardware_interface::CommandInterface> command_interfaces;
        command_interfaces.emplace_back(
            servo_.joint_name_,
            hardware_interface::HW_IF_POSITION,
            &servo_.cmd_
        );
        return command_interfaces;
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
