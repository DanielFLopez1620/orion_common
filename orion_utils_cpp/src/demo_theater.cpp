/*
 * @file demo_theater.cpp
 * @brief Execute a scripted theater performance sequence for ORION demos.
 */

#include <chrono>
#include <thread>

#include "geometry_msgs/msg/twist_stamped.hpp"
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/int32.hpp"
#include "std_msgs/msg/string.hpp"

/* Node that executes a sequence of movements and broadcast messages for a robot performance. */
class RobotPerformer : public rclcpp::Node
{
public:
  /* Initialize cmd_vel, response, and emotion publishers. */
  RobotPerformer()
  : Node("robot_performer_node")
  {
    cmd_vel_pub_ = create_publisher<geometry_msgs::msg::TwistStamped>(
      "/mobile_base_controller/cmd_vel", 10);
    response_pub_ = create_publisher<std_msgs::msg::String>("/orion_response", 10);
    emotion_pub_ = create_publisher<std_msgs::msg::Int32>("/emotion/int", 10);

    RCLCPP_INFO(get_logger(), "Robot Performer Node Initialized.");
  }

  /* Execute the full movement and messaging sequence. */
  void perform_sequence()
  {
    RCLCPP_INFO(get_logger(), "Starting performance sequence...");

    const double FORWARD_SPEED = 2.0;   // m/s
    const double ANGULAR_SPEED = -2.0;  // rad/s (negative for right turn)
    const double SPIN_SPEED = 1.5;      // rad/s (positive for left spin)

    RCLCPP_INFO(get_logger(), "1) Moving forward for 5.0 seconds.");
    publish_for_duration(FORWARD_SPEED, 0.0, 5.0);
    publish_zero();
    RCLCPP_INFO(get_logger(), "Movement 1 completed (Forward).");

    RCLCPP_INFO(get_logger(), "2) Turning right for 1.5 seconds.");
    publish_for_duration(0.0, ANGULAR_SPEED, 1.5);
    publish_zero();
    RCLCPP_INFO(get_logger(), "Movement 2 completed (Right Turn).");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    RCLCPP_INFO(get_logger(), "3) Publishing final response message.");
    std_msgs::msg::String msg;
    msg.data =
      "Buenas noches a todos. Muchas gracias por venir. "
      "Esperamos hayan disfrutado la demo!";
    response_pub_->publish(msg);
    RCLCPP_INFO(get_logger(), "Response published: \"%s\"", msg.data.c_str());

    RCLCPP_INFO(get_logger(), "4) Publishing emotion 7.");
    publish_emotion(7);
    RCLCPP_INFO(get_logger(), "Emotion 7 published.");

    RCLCPP_INFO(get_logger(), "5) Waiting 2 seconds...");
    std::this_thread::sleep_for(std::chrono::seconds(2));

    RCLCPP_INFO(get_logger(), "5) Publishing emotion 3.");
    publish_emotion(3);
    RCLCPP_INFO(get_logger(), "Emotion 3 published.");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    RCLCPP_INFO(get_logger(), "6) Spinning for 1 second.");
    publish_for_duration(0.0, SPIN_SPEED, 1.0);
    publish_zero();
    RCLCPP_INFO(get_logger(), "Movement 6 completed (Spin).");

    RCLCPP_INFO(get_logger(), "Sequence finished. Shutting down.");
  }

private:
  /* Create a TwistStamped message with the given velocities.
   *
   * @param linear_x  Forward linear velocity in m/s.
   * @param angular_z Rotational velocity in rad/s.
   * @return Populated TwistStamped message.
   */
  geometry_msgs::msg::TwistStamped make_twist(double linear_x, double angular_z)
  {
    geometry_msgs::msg::TwistStamped twist;
    twist.header.stamp = get_clock()->now();
    twist.header.frame_id = "base_link";
    twist.twist.linear.x = linear_x;
    twist.twist.angular.z = angular_z;
    return twist;
  }

  /* Publish a velocity command at 20 Hz for the given duration. */
  void publish_for_duration(double linear_x, double angular_z, double seconds)
  {
    auto start = std::chrono::steady_clock::now();
    auto end = start + std::chrono::duration<double>(seconds);

    while (std::chrono::steady_clock::now() < end) {
      cmd_vel_pub_->publish(make_twist(linear_x, angular_z));
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
  }

  /* Publish a zero-velocity stop command. */
  void publish_zero()
  {
    cmd_vel_pub_->publish(make_twist(0.0, 0.0));
  }

  /* Publish an emotion index. */
  void publish_emotion(int value)
  {
    std_msgs::msg::Int32 msg;
    msg.data = value;
    emotion_pub_->publish(msg);
  }

  rclcpp::Publisher<geometry_msgs::msg::TwistStamped>::SharedPtr cmd_vel_pub_;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr response_pub_;
  rclcpp::Publisher<std_msgs::msg::Int32>::SharedPtr emotion_pub_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<RobotPerformer>();
  node->perform_sequence();
  rclcpp::shutdown();
  return 0;
}
