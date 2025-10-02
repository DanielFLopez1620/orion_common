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
    class OrionForwardBridgeNode;

    class ForwardOrion : public hardware_interface::SystemInterface
    {
    public:
        ForwardOrion() = default;

        hardware_interface::CallbackReturn on_init(
            const hardware_interface::HardwareComponentInterfaceParams& params) override;

        hardware_interface::CallbackReturn on_configure(const rclcpp_lifecycle::State& prev_state) override;

        std::vector<hardware_interface::StateInterface> export_state_interfaces() override;

        std::vector<hardware_interface::CommandInterface> export_command_interfaces() override;

        hardware_interface::CallbackReturn on_activate(const rclcpp_lifecycle::State& prev_state) override;

        hardware_interface::CallbackReturn on_deactivate(const rclcpp_lifecycle::State& prev_state) override;

        hardware_interface::return_type read(const rclcpp::Time& time, const rclcpp::Duration& period) override;

        hardware_interface::return_type write(const rclcpp::Time&, const rclcpp::Duration& period) override;

    private:
        std::shared_ptr<OrionForwardBridgeNode> bridge_node_;

        std::string servo_sub_topic_;
        std::string servo_pub_topic_;

        std_msgs::msg::Float32::SharedPtr servo_pose_;
        std_msgs::msg::Float32::SharedPtr servo_cmd_;

        struct ServoComp
        {
            std::string joint_name_;
            double feedback_;
            double cmd_;
        };

        ServoComp servo_;

        rclcpp::Logger logger_{rclcpp::get_logger("ForwardOrion")};

    };

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
    };

}

#endif
