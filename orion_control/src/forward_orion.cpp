#include "orion_control/forward_orion.hpp"
#include <cmath>

namespace orion_control
{
    hardware_interface::CallbackReturn ForwardOrion::on_init(
        const hardware_interface::HardwareComponentInterfaceParams& params)
    {
        if(hardware_interface::SystemInterface::on_init(params) !=
            hardware_interface::CallbackReturn::SUCCESS)
        {
            return hardware_interface::CallbackReturn::ERROR;
        }

        // Loading info from URDF
        auto info_ = params.hardware_info;

        RCLCPP_INFO(this->logger_, "Fwd:: Begin [on_init]...");

        // Shared ptrs as the resources will be used by the hardware interface
        // and the bridge node
        this->servo_pose_ = std::make_shared<std_msgs::msg::Float32>();
        this->servo_cmd_ = std::make_shared<std_msgs::msg::Float32>();

        // Get params of topics from URDF
        const std::string servo_sub_topic =
            info_.hardware_parameters.at("feedback_topic");
        const std::string servo_pub_topic =
            info_.hardware_parameters.at("cmd_topic");
        this->servo_.joint_name_ =
            info_.hardware_parameters.at("servo_name");

        // Lock executor to add the bridge node
        if (auto locked_executor = params.executor.lock())
        {
            this->bridge_node_ = std::make_shared<OrionForwardBridgeNode>(
                info_.name,
                servo_sub_topic,
                servo_pub_topic,
                this->servo_pose_,
                this->servo_cmd_);

            locked_executor->add_node(this->bridge_node_->get_node_base_interface());

            RCLCPP_INFO(this->logger_, "Fwd: Bridge node attached [on_init]...");
        }
        else
        {
            RCLCPP_ERROR(this->logger_, "Fwd: Failed to lock executor [on_init]...");
            return hardware_interface::CallbackReturn::ERROR;
        }

        RCLCPP_INFO(this->logger_, "Fwd:: End [on_init]...");
        return hardware_interface::CallbackReturn::SUCCESS;

    } // on_init()

    hardware_interface::CallbackReturn ForwardOrion::on_configure(
        const rclcpp_lifecycle::State&)
    {
        RCLCPP_INFO(this->logger_, "Fwd:: Begin [on_configure]...");
        RCLCPP_INFO(this->logger_, "Fwd:: End [on_configure]...");

        return hardware_interface::CallbackReturn::SUCCESS;

    } // on_configure()

    std::vector<hardware_interface::StateInterface> ForwardOrion::export_state_interfaces()
    {
        RCLCPP_INFO(this->logger_, "Fwd:: Begin [export_state_interfaces]...");

        std::vector<hardware_interface::StateInterface> state_interfaces;

        state_interfaces.emplace_back(
            hardware_interface::StateInterface(
                this->servo_.joint_name_,
                hardware_interface::HW_IF_POSITION,
                &this->servo_.feedback_
        ));

        RCLCPP_INFO(this->logger_, "Fwd:: End [export_state_interfaces]...");

        return state_interfaces;

    } // export_state_interfaces()

    std::vector<hardware_interface::CommandInterface> ForwardOrion::export_command_interfaces()
    {
        RCLCPP_INFO(this->logger_, "Fwd:: Begin [export_command_interfaces]...");

        std::vector <hardware_interface::CommandInterface> command_interfaces;

        command_interfaces.emplace_back(
            hardware_interface::CommandInterface(this->servo_.joint_name_,
                hardware_interface::HW_IF_POSITION, &this->servo_.cmd_));

        RCLCPP_INFO(this->logger_, "Fwd:: End [export_command_interfaces]...");

        return command_interfaces;

    } // export_command_interfaces()

    hardware_interface::CallbackReturn ForwardOrion::on_activate(
        const rclcpp_lifecycle::State&)
    {
        RCLCPP_INFO(this->logger_, "Fwd:: Begin [on_activate]...");
        if (!this->bridge_node_)
        {
            RCLCPP_ERROR(this->logger_, "Fwd:: Bridge node not initialized — cannot activate.");
            return hardware_interface::CallbackReturn::ERROR;
        }
        RCLCPP_INFO(this->logger_, "Fwd:: End [on_activate]...");
        return hardware_interface::CallbackReturn::SUCCESS;

    } // on_activate()

    hardware_interface::CallbackReturn ForwardOrion::on_deactivate(
        const rclcpp_lifecycle::State&)
    {
        RCLCPP_INFO(this->logger_, "Fwd:: Begin [on_deactivate]...");
        RCLCPP_INFO(this->logger_, "Fwd:: End [on_deactivate]...");
        return hardware_interface::CallbackReturn::SUCCESS;

    } // on_deactivate()

    hardware_interface::return_type ForwardOrion::read(
        const rclcpp::Time&, const rclcpp::Duration&)
    {
        RCLCPP_DEBUG(this->logger_, "Fwd:: Begin [read]...");

        if(this->servo_pose_)
        {
            this->servo_.feedback_ = this->servo_pose_->data;
        }

        RCLCPP_DEBUG(this->logger_, "Fwd:: End [read]...");
        return hardware_interface::return_type::OK;

    } // read()

    hardware_interface::return_type ForwardOrion::write(
        const rclcpp::Time&, const rclcpp::Duration&)
    {
        RCLCPP_DEBUG(this->logger_, "Fwd:: Begin [write]...");

        if(this->servo_cmd_)
        {
            this->servo_cmd_->data = (float) this->servo_.cmd_;
        }

        RCLCPP_DEBUG(this->logger_, "Fwd:: End [write]...");
        return hardware_interface::return_type::OK;

    } // write()

} // orion_control

// ADDING PLUGIN FOR FORWARD CONTROLLER
#include <pluginlib/class_list_macros.hpp>
PLUGINLIB_EXPORT_CLASS(orion_control::ForwardOrion, hardware_interface::SystemInterface)
