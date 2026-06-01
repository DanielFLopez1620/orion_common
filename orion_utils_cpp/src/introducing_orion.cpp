/*
 * @file introducing_orion.cpp
 * @brief Execute an introductory gesture sequence for ORION demos.
 */

#include <chrono>
#include <thread>

#include "geometry_msgs/msg/twist_stamped.hpp"
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/float64_multi_array.hpp"
#include "std_msgs/msg/int32.hpp"

/* Node that executes a scripted gesture sequence to introduce ORION. */
class GestureSequenceNode : public rclcpp::Node
{
public:
  /* Initialize arm, emotion, and cmd_vel publishers, then run the sequence. */
  GestureSequenceNode()
  : Node("gesture_sequence_node")
  {
    arm_pub_ = create_publisher<std_msgs::msg::Float64MultiArray>(
      "/simple_left_arm_controller/commands", 10);
    emotion_pub_ = create_publisher<std_msgs::msg::Int32>("/emotion/int", 10);
    cmd_vel_pub_ = create_publisher<geometry_msgs::msg::TwistStamped>(
      "/mobile_base_controller/cmd_vel", 10);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    run_sequence();
  }

  /* Execute the full introduction gesture sequence and shut down. */
  void run_sequence()
  {
    publish_arm_position(0.0);
    publish_emotion(1);
    publish_cmd_vel_linear(0.2, 1.0);
    publish_emotion(3);

    publish_arm_position(-1.0);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    publish_arm_position(0.0);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    publish_arm_position(-1.0);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    publish_arm_position(0.0);

    RCLCPP_INFO(get_logger(), "Gesture sequence completed. Shutting down...");
    rclcpp::shutdown();
  }

private:
  /* Publish a single arm joint position command.
   *
   * @param position Target arm joint angle in radians.
   */
  void publish_arm_position(double position)
  {
    std_msgs::msg::Float64MultiArray msg;
    msg.data = {position};
    arm_pub_->publish(msg);
    RCLCPP_INFO(get_logger(), "Published arm position: %.2f", position);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }

  /* Publish an emotion index to the emotion topic.
   *
   * @param value Integer emotion identifier to publish.
   */
  void publish_emotion(int value)
  {
    std_msgs::msg::Int32 msg;
    msg.data = value;
    emotion_pub_->publish(msg);
    RCLCPP_INFO(get_logger(), "Published emotion: %d", value);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }

  /* Publish a forward velocity command and then stop the robot.
   *
   * @param linear_x Forward linear velocity in m/s.
   * @param duration  Time in seconds to hold the velocity before stopping.
   */
  void publish_cmd_vel_linear(double linear_x, double duration)
  {
    geometry_msgs::msg::TwistStamped twist;
    twist.twist.linear.x = linear_x;
    cmd_vel_pub_->publish(twist);
    RCLCPP_INFO(get_logger(), "Published forward velocity: %.2f", linear_x);
    std::this_thread::sleep_for(std::chrono::duration<double>(duration));

    // Stop the robot
    twist.twist.linear.x = 0.0;
    cmd_vel_pub_->publish(twist);
    RCLCPP_INFO(get_logger(), "Published stop velocity");
  }

  rclcpp::Publisher<std_msgs::msg::Float64MultiArray>::SharedPtr arm_pub_;
  rclcpp::Publisher<std_msgs::msg::Int32>::SharedPtr emotion_pub_;
  rclcpp::Publisher<geometry_msgs::msg::TwistStamped>::SharedPtr cmd_vel_pub_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<GestureSequenceNode>();
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
