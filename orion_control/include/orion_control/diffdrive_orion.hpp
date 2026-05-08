/*
 * @file diffdrive_orion.hpp
 * @brief ros2_control hardware interface for ORION differential drive base.
 *
 * Bridges the ros2_control DiffDriveController to the µ-ROS topics published
 * and subscribed by orion_ctl_micro_ros: reads wheel encoder counts and writes
 * PWM speed commands via an internal ROS 2 bridge node.
 *
 * Publishes:  /diff_ctl_motor_cmd (Int64MultiArray) — [left_speed, right_speed]
 * Subscribes: /diff_ctl_left_enc, /diff_ctl_right_enc (Int64)
 */

#ifndef DIFFDRIVE_ORION_HPP
#define DIFFDRIVE_ORION_HPP

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

#include <std_msgs/msg/int64_multi_array.hpp>
#include <std_msgs/msg/int64.hpp>

#include "orion_control/wheel.hpp"

namespace orion_control
{
    // Predefinition of the bridge node class
    class OrionDiffBridgeNode;

    /*
     * ros2_control SystemInterface for the ORION differential drive base.
     * Reads encoder feedback and writes motor speed commands via µ-ROS topics.
     */
    class DiffDriveOrion : public hardware_interface::SystemInterface
    {
    public:
        DiffDriveOrion() = default;

        /*
         * Reads wheel and topic params from URDF, allocates shared encoder/command
         * message ptrs, sets up Wheel objects, and attaches the bridge node to the
         * controller executor.
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
         * Exports velocity and position state interfaces for both wheels.
         * @return Vector of [left_vel, left_pos, right_vel, right_pos] interfaces.
         */
        std::vector<hardware_interface::StateInterface> export_state_interfaces() override;

        /*
         * Exports velocity command interfaces for both wheels.
         * @return Vector of [left_cmd_vel, right_cmd_vel] interfaces.
         */
        std::vector<hardware_interface::CommandInterface> export_command_interfaces() override;

        /*
         * Verifies the bridge node is initialized before allowing activation.
         * @return ERROR if bridge node is null, otherwise SUCCESS.
         */
        hardware_interface::CallbackReturn on_activate(
            const rclcpp_lifecycle::State& prev_state) override;

        /*
         * Sends zero velocity to both wheels before releasing control.
         * @return SUCCESS unconditionally.
         */
        hardware_interface::CallbackReturn on_deactivate(
            const rclcpp_lifecycle::State& prev_state) override;

        /*
         * Reads encoder counts from shared ptrs and computes wheel velocity and
         * cumulative position for each wheel.
         * @param period Duration since last read call (used for velocity estimate).
         * @return OK on success.
         */
        hardware_interface::return_type read(
            const rclcpp::Time& time, const rclcpp::Duration& period) override;

        /*
         * Converts rad/s velocity commands to encoder tick rate and publishes
         * them as an Int64MultiArray to the motor command topic.
         * @return OK on success.
         */
        hardware_interface::return_type write(
            const rclcpp::Time&, const rclcpp::Duration& period) override;

    private:
        std::shared_ptr<OrionDiffBridgeNode> bridge_node_;

        std_msgs::msg::Int64MultiArray::SharedPtr cmd_speed_;
        std_msgs::msg::Int64::SharedPtr left_enc_;
        std_msgs::msg::Int64::SharedPtr right_enc_;

        struct Config
        {
            std::string left_wheel_name {"left_wheel"};
            std::string right_wheel_name {"right_wheel"};
            int enc_tics_per_rev {0};
            std::string motor_cmd_topic {"/diff_ctl_motor_cmd"};
            std::string left_enc_topic {"/diff_ctl_left_enc"};
            std::string right_enc_topic {"/diff_ctl_right_enc"};
        };

        Config config_;

        Wheel left_wheel_;
        Wheel right_wheel_;

        rclcpp::Logger logger_{rclcpp::get_logger("DiffDriveOrion")};

    }; // class DiffDriveOrion

    /*
     * Internal bridge node that shuttles data between the hardware interface and
     * the µ-ROS encoder/command topics at 20 Hz (50 ms timer).
     */
    class OrionDiffBridgeNode : public rclcpp::Node
    {
    public:
        OrionDiffBridgeNode(
            const std::string& name,
            const std::string& motor_cmd_topic,
            const std::string& left_enc_topic,
            const std::string& right_enc_topic,
            std_msgs::msg::Int64::SharedPtr left_enc,
            std_msgs::msg::Int64::SharedPtr right_enc,
            std_msgs::msg::Int64MultiArray::SharedPtr cmd_speed)
        : rclcpp::Node("orion_diff_bridge_node_" + name),
          left_enc_ptr_(left_enc),
          right_enc_ptr_(right_enc),
          cmd_speed_ptr_(cmd_speed)
        {
            this->pub_cmd_speed_ = this->create_publisher<std_msgs::msg::Int64MultiArray>(
                motor_cmd_topic, rclcpp::QoS(1));

            this->pub_timer_= this->create_wall_timer(
                std::chrono::milliseconds(50),
                std::bind(&OrionDiffBridgeNode::publish_commands, this)
            );

            this->sub_left_enc_ = this->create_subscription<std_msgs::msg::Int64>(
                left_enc_topic, rclcpp::QoS(1),
                [this](const std_msgs::msg::Int64::SharedPtr enc_pos)
                {
                    left_enc_ptr_->data = enc_pos->data;
                }
            );

            this->sub_right_enc_ = this->create_subscription<std_msgs::msg::Int64>(
                right_enc_topic, rclcpp::QoS(1),
                [this](const std_msgs::msg::Int64::SharedPtr enc_pos)
                {
                    right_enc_ptr_->data = enc_pos->data;
                }
            );
        }
    private:
        void publish_commands()
        {
            if(rclcpp::ok() && cmd_speed_ptr_)
            {
                this->pub_cmd_speed_->publish(*cmd_speed_ptr_);
            }
        }

        std_msgs::msg::Int64::SharedPtr left_enc_ptr_;
        std_msgs::msg::Int64::SharedPtr right_enc_ptr_;
        std_msgs::msg::Int64MultiArray::SharedPtr cmd_speed_ptr_;

        rclcpp::Subscription<std_msgs::msg::Int64>::SharedPtr sub_left_enc_;
        rclcpp::Subscription<std_msgs::msg::Int64>::SharedPtr sub_right_enc_;
        rclcpp::Publisher<std_msgs::msg::Int64MultiArray>::SharedPtr pub_cmd_speed_;
        rclcpp::TimerBase::SharedPtr pub_timer_;

    }; // class OrionDiffBridgeNode

} // orion_control

#endif
