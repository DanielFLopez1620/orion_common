#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/float64.hpp>
#include "orion_control/orion_shared_state.hpp"

class OrionBridgeForward : public rclcpp::Node
{
public:
    OrionBridgeForward() : Node("orion_bridge_forward")
    {
        this->declare_parameter<std::string>("servo_left_feedback", "/fwd_servo_left_feedback");
        this->declare_parameter<std::string>("servo_left_cmd", "/fwd_servo_left_cmd");
        this->declare_parameter<std::string>("servo_right_feedback", "/fwd_servo_right_feedback");
        this->declare_parameter<std::string>("servo_right_cmd", "/fwd_servo_right_cmd");

        auto sl_feedback_topic = this->get_parameter("servo_left_feedback").as_string();
        auto sl_cmd_topic = this->get_parameter("servo_left_cmd").as_string();
        auto sr_feedback_topic = this->get_parameter("servo_right_feedback").as_string();
        auto sr_cmd_topic = this->get_parameter("servo_right_cmd").as_string();

        servo_left_sub_ = this->create_subscription<std_msgs::msg::Float64>(sl_feedback_topic, 10,
            [](std_msgs::msg::Float64::SharedPtr msg)
            {
                g_orion_forw_state.servos[0].pos.store(msg->data);
            }
        );

        servo_right_sub_ = this->create_subscription<std_msgs::msg::Float64>(sr_feedback_topic, 10,
            [](std_msgs::msg::Float64::SharedPtr msg)
            {
                g_orion_forw_state.servos[1].pos.store(msg->data);
            }
        );

        servo_left_pub_ = this->create_publisher<std_msgs::msg::Float64>(sl_cmd_topic, 10);
        servo_right_pub_ = this->create_publisher<std_msgs::msg::Float64>(sr_cmd_topic, 10);

        timer_ = this->create_wall_timer(std::chrono::milliseconds(20),
            [this]() { this->publish_servo_cmd(); });
    }
private:
    void publish_servo_cmd()
    {
        auto cmd_left = std_msgs::msg::Float64();
        cmd_left.data = g_orion_forw_state.servos[0].cmd.load();
        servo_left_pub_->publish(cmd_left);

        auto cmd_right = std_msgs::msg::Float64();
        cmd_right.data = g_orion_forw_state.servos[1].cmd.load();
        servo_right_pub_->publish(cmd_right);
    }

    rclcpp::Subscription<std_msgs::msg::Float64>::SharedPtr servo_left_sub_;
    rclcpp::Subscription<std_msgs::msg::Float64>::SharedPtr servo_right_sub_;
    rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr servo_left_pub_;
    rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr servo_right_pub_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<OrionBridgeForward>());
    rclcpp::shutdown();
    return 0;
}