#include "orion_control/forward_orion.hpp"
#include <cmath>

namespace orion_control
{
    /**
     * Forward control actions to implement on initialization of the controller
     * that includes the set up of the servo to command, the locking of the
     * executor to add the node that interacts with µ-ROS reading and
     * writing methods and the callback return definitions.
     *
     * @param params Parameters required for the initialization of a hardware
     *      interface component.
     *
     * @return ERROR if params were not validated or if it was not possible to
     *      add the bridge node, otherwise SUCCESS
     */
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

    /**
     * For now, just used to log that that configure was passed.
     *
     * @return Success if the on_configure was passed without any issues.
     */
    hardware_interface::CallbackReturn ForwardOrion::on_configure(
        const rclcpp_lifecycle::State&)
    {
        RCLCPP_INFO(this->logger_, "Fwd:: Begin [on_configure]...");
        RCLCPP_INFO(this->logger_, "Fwd:: End [on_configure]...");

        return hardware_interface::CallbackReturn::SUCCESS;

    } // on_configure()

    /**
     * Expose the read-only variables for feedback on the control process.
     *
     * @return Vector of the state interfaces used, in this case the servo
     *      feedback position.
     */
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

    /**
     * Expose the writable variables for commands, in this case, servo position.
     *
     * @return Vector of the command interfaces used, in this case, servo
     *      command position.
     */
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

    /**
     * For now just used to log that activate was passed.
     *
     * @return Success if on_activate was completed safely.
     */
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

    /**
     * Holds servo at current position on deactivation — no zero-position command
     * to avoid moving arms to an unsafe pose.
     *
     * @return Success if deactivate was completed safely.
     */
    hardware_interface::CallbackReturn ForwardOrion::on_deactivate(
        const rclcpp_lifecycle::State&)
    {
        RCLCPP_INFO(this->logger_, "Fwd:: Begin [on_deactivate]...");
        RCLCPP_INFO(this->logger_, "Fwd:: End [on_deactivate]...");
        return hardware_interface::CallbackReturn::SUCCESS;

    } // on_deactivate()

    /**
     * Read the sensor (servo position) updates and stores its value, where
     * the position is read in radians.
     *
     * @param time [Unused] stores the time when called
     * @param duration [Unused] Stores the duration (period) of the read
     *
     * @return OK if the reading process was completely safely. Otherwise,
     *      it will raise an error.
     */
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

    /**
     * Write to the actuator (servo position) to command the objective
     * received, where the position is in radians.
     *
     * @param time [Unused] stores the time when called
     * @param duration  [Unused] Stores the duration (period) of the read
     *
     * @return OK if the writing process was completely safely. Otherwise,
     *      it will raise an error.
     */
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