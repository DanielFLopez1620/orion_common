/*
 * @file check_mov.cpp
 * @brief Measure cmd_vel-to-odometry latency for ORION mobile base diagnostics.
 */

#include "geometry_msgs/msg/twist_stamped.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include "rclcpp/rclcpp.hpp"

static constexpr double THRESHOLD = 0.02;  // m/s minimum linear velocity to detect movement

/* Node that measures latency between a velocity command and the first observed movement. */
class LatencyMeasure : public rclcpp::Node
{
public:
  /* Subscribe to cmd_vel and odom to record timestamps for latency calculation. */
  LatencyMeasure()
  : Node("latency_measure_node"),
    cmd_vel_time_(0.0),
    measured_(false)
  {
    sub_cmd_ = create_subscription<geometry_msgs::msg::TwistStamped>(
      "/mobile_base_controller/cmd_vel", 10,
      [this](const geometry_msgs::msg::TwistStamped::SharedPtr msg) {
        cmd_callback(msg);
      });

    sub_odom_ = create_subscription<nav_msgs::msg::Odometry>(
      "/mobile_base_controller/odom", 10,
      [this](const nav_msgs::msg::Odometry::SharedPtr msg) {
        odom_callback(msg);
      });
  }

private:
  /* Record the timestamp when a velocity command is observed.
   *
   * @param msg TwistStamped velocity command message.
   */
  void cmd_callback(const geometry_msgs::msg::TwistStamped::SharedPtr /*msg*/)
  {
    cmd_vel_time_ = get_clock()->now().nanoseconds() / 1e9;
    RCLCPP_INFO(get_logger(), "[CMD_VEL] Sent to %.4f s", cmd_vel_time_);
  }

  /* Compute latency when movement above threshold is first detected.
   *
   * @param msg Odometry message.
   */
  void odom_callback(const nav_msgs::msg::Odometry::SharedPtr msg)
  {
    if (!measured_ && cmd_vel_time_ > 0.0) {
      double vx = msg->twist.twist.linear.x;
      if (std::abs(vx) > THRESHOLD) {
        double move_time = get_clock()->now().nanoseconds() / 1e9;
        double latency = move_time - cmd_vel_time_;
        RCLCPP_INFO(get_logger(), "[ODOM] detected move %.4f s", move_time);
        RCLCPP_INFO(get_logger(), "Total latency: %.4f s", latency);
        measured_ = true;
      }
    }
  }

  double cmd_vel_time_;
  bool measured_;

  rclcpp::Subscription<geometry_msgs::msg::TwistStamped>::SharedPtr sub_cmd_;
  rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr sub_odom_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<LatencyMeasure>());
  rclcpp::shutdown();
  return 0;
}
