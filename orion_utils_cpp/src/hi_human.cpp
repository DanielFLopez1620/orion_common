/*
 * @file hi_human.cpp
 * @brief Execute a greeting sequence for ORION demos with motion and arm gestures.
 */

#include <chrono>
#include <thread>

#include "geometry_msgs/msg/twist_stamped.hpp"
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/float64_multi_array.hpp"
#include "std_msgs/msg/int32.hpp"

/* Node that publishes a greeting sequence of emotions, motion, arm waves, and rotation. */
class OrionSimpleSequence : public rclcpp::Node
{
public:
  /* Initialize cmd_vel, left arm, and emotion publishers. */
  OrionSimpleSequence()
  : Node("orion_simple_sequence")
  {
    cmd_vel_pub_ = create_publisher<geometry_msgs::msg::TwistStamped>(
      "/mobile_base_controller/cmd_vel", 10);
    left_arm_pub_ = create_publisher<std_msgs::msg::Float64MultiArray>(
      "/simple_left_arm_controller/commands", 10);
    emotion_pub_ = create_publisher<std_msgs::msg::Int32>("/emotion/int", 10);

    RCLCPP_INFO(get_logger(), "ORION simple sequence node started");
  }

  /* Publish an emotion index to the emotion topic.
   *
   * @param emotion_id Integer emotion identifier to publish.
   */
  void publish_emotion(int emotion_id)
  {
    std_msgs::msg::Int32 msg;
    msg.data = emotion_id;
    emotion_pub_->publish(msg);
    RCLCPP_INFO(get_logger(), "Emotion set to %d", emotion_id);
  }

  /* Publish forward velocity at the given rate, then stop.
   *
   * @param speed    Forward linear velocity in m/s.
   * @param duration Time in seconds to move before stopping.
   * @param hz       Publishing rate in Hz.
   */
  void move_forward(double speed, double duration, double hz = 20.0)
  {
    geometry_msgs::msg::TwistStamped twist;
    twist.twist.linear.x = speed;
    twist.twist.angular.z = 0.0;

    auto period_ms = static_cast<int>(1000.0 / hz);
    auto start = std::chrono::steady_clock::now();
    auto end_time = start + std::chrono::duration<double>(duration);

    while (std::chrono::steady_clock::now() < end_time && rclcpp::ok()) {
      twist.header.stamp = get_clock()->now();
      cmd_vel_pub_->publish(twist);
      std::this_thread::sleep_for(std::chrono::milliseconds(period_ms));
    }

    // Stop robot
    twist.twist.linear.x = 0.0;
    cmd_vel_pub_->publish(twist);
  }

  /* Move the left arm from start to end position and back.
   *
   * @param start Initial and final joint angle in radians.
   * @param end   Target joint angle in radians for the wave peak.
   */
  void wave_left_arm(double start = 0.0, double end = 1.57)
  {
    std_msgs::msg::Float64MultiArray msg;

    msg.data = {start};
    left_arm_pub_->publish(msg);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    msg.data = {end};
    left_arm_pub_->publish(msg);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    msg.data = {start};
    left_arm_pub_->publish(msg);
    RCLCPP_INFO(get_logger(), "Left arm greeting executed");
  }

  /* Publish rightward rotation commands at the given rate, then stop.
   *
   * @param angular_speed Magnitude of angular velocity in rad/s (applied as negative).
   * @param duration      Time in seconds to rotate before stopping.
   * @param hz            Publishing rate in Hz.
   */
  void rotate_right(double angular_speed, double duration, double hz = 20.0)
  {
    geometry_msgs::msg::TwistStamped twist;
    twist.twist.linear.x = 0.0;
    twist.twist.angular.z = -angular_speed;  // negative = right

    auto period_ms = static_cast<int>(1000.0 / hz);
    auto start = std::chrono::steady_clock::now();
    auto end_time = start + std::chrono::duration<double>(duration);

    while (std::chrono::steady_clock::now() < end_time && rclcpp::ok()) {
      twist.header.stamp = get_clock()->now();
      cmd_vel_pub_->publish(twist);
      std::this_thread::sleep_for(std::chrono::milliseconds(period_ms));
    }

    // Stop rotation
    twist.twist.angular.z = 0.0;
    cmd_vel_pub_->publish(twist);
  }

private:
  rclcpp::Publisher<geometry_msgs::msg::TwistStamped>::SharedPtr cmd_vel_pub_;
  rclcpp::Publisher<std_msgs::msg::Float64MultiArray>::SharedPtr left_arm_pub_;
  rclcpp::Publisher<std_msgs::msg::Int32>::SharedPtr emotion_pub_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<OrionSimpleSequence>();

  // Small delay to ensure publishers are ready
  std::this_thread::sleep_for(std::chrono::seconds(1));

  // 1. Set emotion
  node->publish_emotion(4);
  node->publish_emotion(4);
  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  // 2. Move forward for 1.5 seconds at 0.75 m/s
  node->move_forward(1.5, 0.75, 20.0);

  // 3. Left arm greeting
  node->wave_left_arm(0.0, -1.57);
  node->wave_left_arm(0.0, -1.57);
  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  node->publish_emotion(7);
  node->publish_emotion(7);
  node->publish_emotion(7);
  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  // 4. Rotate right at 0.5 rad/s for 1.5 seconds
  node->rotate_right(2.5, 4.0, 20.0);

  RCLCPP_INFO(node->get_logger(), "ORION sequence finished");

  rclcpp::shutdown();
  return 0;
}
