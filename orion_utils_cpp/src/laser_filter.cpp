// Copyright 2025 DanielFLopez1620
//
// Licensed under the BSD-3-Clause License.

/*
 * @file laser_filter.cpp
 * @brief Filter LIDAR scan ranges to remove robot self-obstruction areas.
 */

#include <cmath>
#include <limits>
#include <vector>

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/laser_scan.hpp"

/* Node that filters angle-based scan ranges from a LIDAR topic.
 *
 * Subscribes to '/ldlidar_node/scan', sets ranges within configurable
 * angle intervals to NaN, and publishes the result to '/scan_filtered'.
 */
class LaserFilterNode : public rclcpp::Node
{
public:
  /* Initialize the laser filter node with parameters and pub/sub. */
  LaserFilterNode()
  : Node("laser_filter_node")
  {
    declare_parameter(
      "filter_ranges", std::vector<double>{
            0.5566728, 0.9268571,
            2.2142123, 2.6901637,
            3.6979158, 4.1386122,
            5.3913217, 5.7438788
      });

    auto flat = get_parameter("filter_ranges").as_double_array();
    for (size_t i = 0; i + 1 < flat.size(); i += 2) {
      filter_ranges_.emplace_back(flat[i], flat[i + 1]);
    }

    sub_ = create_subscription<sensor_msgs::msg::LaserScan>(
      "/ldlidar_node/scan", 10,
      [this](const sensor_msgs::msg::LaserScan::SharedPtr msg) {
        scan_callback(msg);
      });

    pub_ = create_publisher<sensor_msgs::msg::LaserScan>("/scan_filtered", 10);
  }

private:
  /* Receive a LaserScan, zero out filtered angle ranges, and republish.
   *
   * @param msg Unfiltered LaserScan message received from the LIDAR.
   */
  void scan_callback(const sensor_msgs::msg::LaserScan::SharedPtr msg)
  {
    auto filtered = *msg;

    for (size_t i = 0; i < filtered.ranges.size(); ++i) {
      float angle = msg->angle_min + static_cast<float>(i) * msg->angle_increment;

      for (const auto & [lower, upper] : filter_ranges_) {
        if (angle >= static_cast<float>(lower) && angle <= static_cast<float>(upper)) {
          filtered.ranges[i] = std::numeric_limits<float>::quiet_NaN();
          break;
        }
      }
    }

    pub_->publish(filtered);
  }

  std::vector<std::pair<double, double>> filter_ranges_;
  rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr sub_;
  rclcpp::Publisher<sensor_msgs::msg::LaserScan>::SharedPtr pub_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<LaserFilterNode>());
  rclcpp::shutdown();
  return 0;
}
