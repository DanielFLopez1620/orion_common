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

    /**
     * Class oriented to implement the control for a differential controller
     * aiming to related the lecture of the encoders and the PWM commands.
     */
    class DiffDriveOrion : public hardware_interface::SystemInterface
    {
    public:
        DiffDriveOrion() = default;

        hardware_interface::CallbackReturn on_init(
            const hardware_interface::HardwareComponentInterfaceParams& params) override;

        hardware_interface::CallbackReturn on_configure(
            const rclcpp_lifecycle::State& prev_state) override;

        std::vector<hardware_interface::StateInterface> export_state_interfaces() override;

        std::vector<hardware_interface::CommandInterface> export_command_interfaces() override;

        hardware_interface::CallbackReturn on_activate(
            const rclcpp_lifecycle::State& prev_state) override;

        hardware_interface::CallbackReturn on_deactivate(
            const rclcpp_lifecycle::State& prev_state) override;

        hardware_interface::return_type read(
            const rclcpp::Time& time, const rclcpp::Duration& period) override;

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
        };

        Config config_;

        Wheel left_wheel_;
        Wheel right_wheel_;

        rclcpp::Logger logger_{rclcpp::get_logger("DiffDriveOrion")};

    }; // class DiffDriveOrion

    /**
     * Node class oriented to implement the bridge communication between the
     * hardware interface and the µ-ROS communication.
     */
    class OrionDiffBridgeNode : public rclcpp::Node
    {
    public:
        OrionDiffBridgeNode(
            const std::string& name,
            std_msgs::msg::Int64::SharedPtr left_enc,
            std_msgs::msg::Int64::SharedPtr right_enc,
            std_msgs::msg::Int64MultiArray::SharedPtr cmd_speed)
        : rclcpp::Node("orion_diff_bridge_node_" + name),
          left_enc_ptr_(left_enc),
          right_enc_ptr_(right_enc),
          cmd_speed_ptr_(cmd_speed)
        {
            this->pub_cmd_speed_ = this->create_publisher<std_msgs::msg::Int64MultiArray>(
                "/diff_ctl_motor_cmd", rclcpp::QoS(1));

            this->pub_timer_= this->create_wall_timer(
                std::chrono::milliseconds(100),
                std::bind(&OrionDiffBridgeNode::publish_commands, this)
            );

            this->sub_left_enc_ = this->create_subscription<std_msgs::msg::Int64>(
                "/diff_ctl_left_enc", rclcpp::QoS(1),
                [this](const std_msgs::msg::Int64::SharedPtr enc_pos)
                {
                    left_enc_ptr_->data = enc_pos->data;
                }
            );

            this->sub_right_enc_ = this->create_subscription<std_msgs::msg::Int64>(
                "/diff_ctl_right_enc", rclcpp::QoS(1),
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
