/*
 * @file happy_birthday.cpp
 * @brief Execute a birthday greeting sequence with arm oscillation and TTS for ORION.
 */

#include <chrono>
#include <cmath>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/float64_multi_array.hpp"
#include "std_msgs/msg/int64.hpp"
#include "std_msgs/msg/string.hpp"

/* Node that publishes a birthday greeting with arm oscillation and TTS for ORION. */
class OrionCelebrator : public rclcpp::Node
{
public:
  /* Initialize publishers, send greeting message, and start arm timer. */
  OrionCelebrator()
  : Node("orion_celebrator")
  {
    emotion_pub_ = create_publisher<std_msgs::msg::Int64>("/emotion/int", 10);
    arm_right_pub_ = create_publisher<std_msgs::msg::Float64MultiArray>(
      "/simple_right_arm_controller/commands", 10);
    arm_left_pub_ = create_publisher<std_msgs::msg::Float64MultiArray>(
      "/simple_left_arm_controller/commands", 10);
    tts_pub_ = create_publisher<std_msgs::msg::String>("/orion_response", 10);

    // Publish emotion once at start
    std_msgs::msg::Int64 emotion_msg;
    emotion_msg.data = 1;
    emotion_pub_->publish(emotion_msg);
    RCLCPP_INFO(get_logger(), "Emotion published: 1");

    // Publish creative birthday message
    std_msgs::msg::String bday_msg;
    bday_msg.data =
      "¡Feliz cumpleaños! Que tus circuitos brillen más que mis LEDs, "
      "y que tengas un año lleno de aventuras épicas.";
    tts_pub_->publish(bday_msg);
    RCLCPP_INFO(get_logger(), "TTS message published: %s", bday_msg.data.c_str());

    start_time_ = now();

    // 10 Hz timer for arm waving
    timer_ = create_wall_timer(
      std::chrono::milliseconds(100),
      [this]() {arm_motion_callback();});
  }

private:
  /* Oscillate both arms between -1 and 1 rad using a sine wave. */
  void arm_motion_callback()
  {
    double elapsed = (now() - start_time_).seconds();
    double angle = std::sin(elapsed * 2.0);

    std_msgs::msg::Float64MultiArray msg_right;
    msg_right.data = {angle};
    arm_right_pub_->publish(msg_right);

    std_msgs::msg::Float64MultiArray msg_left;
    msg_left.data = {-angle};
    arm_left_pub_->publish(msg_left);
  }

  rclcpp::Time start_time_;

  rclcpp::Publisher<std_msgs::msg::Int64>::SharedPtr emotion_pub_;
  rclcpp::Publisher<std_msgs::msg::Float64MultiArray>::SharedPtr arm_right_pub_;
  rclcpp::Publisher<std_msgs::msg::Float64MultiArray>::SharedPtr arm_left_pub_;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr tts_pub_;
  rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<OrionCelebrator>();
  try {
    rclcpp::spin(node);
  } catch (const std::exception &) {
    // intentional interrupt
  }
  rclcpp::shutdown();
  return 0;
}
