#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/int64.hpp>
#include <std_msgs/msg/int64_multi_array.hpp>
#include <orion_control/orion_shared_state.hpp>

class OrionBridgeUros : public rclcpp::Node
{
public:
    OrionBridgeUros() : Node("orion_bridge_uros")
    {
        sub_enc_left_ = this->create_subscription<std_msgs::msg::Int64>(
            "/diff_ctl_left_enc", 10,
            [this](const std_msgs::msg::Int64::SharedPtr msg)
            {
                g_orion_diff_state.enc_left.store(msg->data, std::memory_order_relaxed);
            }
        );

        sub_enc_right_ = this->create_subscription<std_msgs::msg::Int64>(
            "/diff_ctl_right_enc", 10,
            [this](const std_msgs::msg::Int64::SharedPtr msg)
            {
                g_orion_diff_state.enc_right.store(msg->data, std::memory_order_relaxed);
            }
        );

        pub_motor_cmd_= this->create_publisher<std_msgs::msg::Int64MultiArray>(
            "/diff_ctl_motor_cmd", 10);

        timer_ = this->create_wall_timer(
            std::chrono::milliseconds(20),
            std::bind(&OrionBridgeUros::publish_motors_cmd, this)
        );
    }

private:
    void publish_motors_cmd()
    {
        auto msg = std_msgs::msg::Int64MultiArray();
        msg.data.resize(2);

        msg.data[0] = g_orion_diff_state.cmd_left.load(std::memory_order_relaxed);
        msg.data[1] = g_orion_diff_state.cmd_right.load(std::memory_order_relaxed);
    }

    rclcpp::Subscription<std_msgs::msg::Int64>::SharedPtr sub_enc_left_;
    rclcpp::Subscription<std_msgs::msg::Int64>::SharedPtr sub_enc_right_;
    rclcpp::Publisher<std_msgs::msg::Int64MultiArray>::SharedPtr pub_motor_cmd_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<OrionBridgeUros>());
    rclcpp::shutdown();
    return 0;
}