// Copyright 2025 DanielFLopez1620
//
// Licensed under the BSD-3-Clause License.

/*
 * @file emotion_try.cpp
 * @brief Test emotion expressions and arm motion across all ORION emotion indices.
 */

#include <chrono>
#include <cmath>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/float64_multi_array.hpp"
#include "std_msgs/msg/int32.hpp"

/* Node that cycles through ORION emotion expressions with synchronized arm sine motion. */
class OrionEmotionTest : public rclcpp::Node
{
public:
  /* Initialize publishers, emotion counter, and 50 Hz update timer. */
  OrionEmotionTest()
  : Node("orion_emotion_test"),
    current_emotion_(0)
  {
    pub_emotion_ = create_publisher<std_msgs::msg::Int32>("/emotion/int", 10);
    pub_left_arm_ = create_publisher<std_msgs::msg::Float64MultiArray>(
      "/simple_left_arm_controller/commands", 10);
    pub_right_arm_ = create_publisher<std_msgs::msg::Float64MultiArray>(
      "/simple_right_arm_controller/commands", 10);

    last_emotion_time_ = now();
    start_time_ = now();

    timer_ = create_wall_timer(
      std::chrono::milliseconds(20),
      [this]() {update_loop();});

    RCLCPP_INFO(get_logger(), "OrionEmotionTest node started.");
  }

private:
  /* Advance emotion counter and publish synchronized arm sine motion. */
  void update_loop()
  {
    auto current_time = now();

    // Cycle emotion every emotion_interval_ seconds
    if ((current_time - last_emotion_time_).seconds() >= emotion_interval_) {
      current_emotion_ = (current_emotion_ + 1) % (max_emotion_ + 1);

      std_msgs::msg::Int32 msg;
      msg.data = current_emotion_;
      pub_emotion_->publish(msg);

      RCLCPP_INFO(get_logger(), "[EMOTION] Changed to %d", current_emotion_);
      last_emotion_time_ = current_time;
    }

    // Arm sine wave keyed to elapsed time modulo arm_period_
    double elapsed = std::fmod((current_time - start_time_).seconds(), arm_period_);
    double phase = 2.0 * M_PI * elapsed / arm_period_;
    double angle = arm_amplitude_ * std::sin(phase);

    std_msgs::msg::Float64MultiArray arm_msg;
    arm_msg.data = {angle};
    pub_left_arm_->publish(arm_msg);
    pub_right_arm_->publish(arm_msg);
  }

  int current_emotion_;
  const int max_emotion_{7};
  const double emotion_interval_{2.0};
  const double arm_period_{5.0};
  const double arm_amplitude_{1.0};

  rclcpp::Time last_emotion_time_;
  rclcpp::Time start_time_;

  rclcpp::Publisher<std_msgs::msg::Int32>::SharedPtr pub_emotion_;
  rclcpp::Publisher<std_msgs::msg::Float64MultiArray>::SharedPtr pub_left_arm_;
  rclcpp::Publisher<std_msgs::msg::Float64MultiArray>::SharedPtr pub_right_arm_;
  rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<OrionEmotionTest>();
  try {
    rclcpp::spin(node);
  } catch (const std::exception & e) {
    RCLCPP_INFO(node->get_logger(), "Stopping OrionEmotionTest node...");
  }
  rclcpp::shutdown();
  return 0;
}
