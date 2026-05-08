/*
 * @file forward_orion.hpp
 * @brief ros2_control hardware interface for ORION servo arm controllers.
 *
 * Bridges the ros2_control ForwardCommandController to the µ-ROS topics
 * published and subscribed by orion_ctl_micro_ros: reads servo position
 * feedback and writes position commands via an internal ROS 2 bridge node.
 *
 * Publishes:  feedback_topic (Float32) — servo position feedback (radians)
 * Subscribes: cmd_topic      (Float32) — servo position command  (radians)
 * Topics are configured per-instance via URDF hardware parameters.
 */

#ifndef FORWARD_ORION_HPP
#define FORWARD_ORION_HPP

#include <string>
#include <vector>

#include <hardware_interface/handle.hpp>
#include <hardware_interface/hardware_info.hpp>
#include <hardware_interface/system_interface.hpp>
#include <hardware_interface/types/hardware_interface_return_values.hpp>
#include <hardware_interface/types/hardware_interface_type_values.hpp>

#include <rclcpp/rclcpp.hpp>
#include <rclcpp_lifecycle/node_interfaces/lifecycle_node_interface.hpp>
#include <rclcpp_lifecycle/state.hpp>
#include <rclcpp/publisher.hpp>
#include <rclcpp/subscription.hpp>

#include <std_msgs/msg/float32.hpp>

namespace orion_control
{
    // Predefinition of the bridge class
    class OrionForwardBridgeNode;

    /*
     * ros2_control SystemInterface for ORION servo arms (MG996R, MG995, MG90, SG90).
     * Reads position feedback and writes position commands via µ-ROS topics.
     */
    class ForwardOrion : public hardware_interface::SystemInterface
    {
    public:
        ForwardOrion() = default;

        /*
         * Reads servo and topic params from URDF, allocates shared pose/command
         * message ptrs, and attaches the bridge node to the controller executor.
         * @return ERROR if parent init or executor lock fails, otherwise SUCCESS.
         */
        hardware_interface::CallbackReturn on_init(
            const hardware_interface::HardwareComponentInterfaceParams& params) override;

        /*
         * Lifecycle configure transition — currently a no-op beyond logging.
         * @return SUCCESS unconditionally.
         */
        hardware_interface::CallbackReturn on_configure(
            const rclcpp_lifecycle::State& prev_state) override;

        /*
         * Exports the servo position feedback state interface.
         * @return Vector containing the single HW_IF_POSITION state interface.
         */
        std::vector<hardware_interface::StateInterface> export_state_interfaces() override;

        /*
         * Exports the servo position command interface.
         * @return Vector containing the single HW_IF_POSITION command interface.
         */
        std::vector<hardware_interface::CommandInterface> export_command_interfaces() override;

        /*
         * Verifies the bridge node is initialized before allowing activation.
         * @return ERROR if bridge node is null, otherwise SUCCESS.
         */
        hardware_interface::CallbackReturn on_activate(
            const rclcpp_lifecycle::State& prev_state) override;

        /*
         * Holds servo at its current position on deactivation — no zero command
         * is sent to avoid moving the arm to an unsafe pose.
         * @return SUCCESS unconditionally.
         */
        hardware_interface::CallbackReturn on_deactivate(
            const rclcpp_lifecycle::State& prev_state) override;

        /*
         * Copies servo position feedback from the shared ptr into the state variable.
         * Position is in radians.
         * @return OK on success.
         */
        hardware_interface::return_type read(
            const rclcpp::Time& time, const rclcpp::Duration& period) override;

        /*
         * Copies the position command into the shared ptr for the bridge to publish.
         * Position is in radians.
         * @return OK on success.
         */
        hardware_interface::return_type write(
            const rclcpp::Time&, const rclcpp::Duration& period) override;

    private:
        std::shared_ptr<OrionForwardBridgeNode> bridge_node_;

        std_msgs::msg::Float32::SharedPtr servo_pose_;
        std_msgs::msg::Float32::SharedPtr servo_cmd_;

        std::string servo_sub_topic_;
        std::string servo_pub_topic_;

        struct ServoComp
        {
            std::string joint_name_;
            double feedback_;
            double cmd_;
        };

        ServoComp servo_;

        rclcpp::Logger logger_{rclcpp::get_logger("ForwardOrion")};

    }; // class ForwardOrion

    /*
     * Internal bridge node that shuttles data between the hardware interface and
     * the µ-ROS servo topics at 10 Hz (100 ms timer).
     */
    class OrionForwardBridgeNode : public rclcpp::Node
    {
    public:
        OrionForwardBridgeNode(
            const std::string& name,
            const std::string& sub_topic,
            const std::string& pub_topic,
            std_msgs::msg::Float32::SharedPtr servo_pose_ptr,
            std_msgs::msg::Float32::SharedPtr servo_cmd_ptr)
        : rclcpp::Node("orion_fwd_bridge_node_" + name),
          servo_pose_ptr_(servo_pose_ptr),
          servo_cmd_ptr_(servo_cmd_ptr)
        {
            this->servo_cmd_pub_ = this->create_publisher<std_msgs::msg::Float32>(
                pub_topic, rclcpp::QoS(1));

            this->pub_timer_ = this->create_wall_timer(
                std::chrono::milliseconds(100),
                std::bind(&OrionForwardBridgeNode::publish_commands, this));

            this->servo_pose_sub_ = this->create_subscription<std_msgs::msg::Float32>(
                sub_topic, rclcpp::QoS(1),
                [this](const std_msgs::msg::Float32::SharedPtr pose)
                {
                    if(servo_pose_ptr_)
                    {
                        servo_pose_ptr_->data = pose->data;
                    }
                });
            }
    private:
        /* Publishes the current servo command to the µ-ROS cmd topic. */
        void publish_commands()
        {
            if(rclcpp::ok() && servo_cmd_ptr_)
            {
                this->servo_cmd_pub_->publish(*servo_cmd_ptr_);
            }
        }

        std_msgs::msg::Float32::SharedPtr servo_pose_ptr_;
        std_msgs::msg::Float32::SharedPtr servo_cmd_ptr_;

        rclcpp::Subscription<std_msgs::msg::Float32>::SharedPtr servo_pose_sub_;
        rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr servo_cmd_pub_;
        rclcpp::TimerBase::SharedPtr pub_timer_;

    }; // class OrionForwardBridgeNode

} // orion_control

#endif
