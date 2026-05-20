#!/usr/bin/env python3
"""Filter LIDAR scan ranges to remove robot self-obstruction areas."""

import rclpy
from rclpy.node import Node
from sensor_msgs.msg import LaserScan


class LaserFilterNode(Node):
    """Filter angle-based scan ranges from a LIDAR topic.

    Subscribes to '/ldlidar_node/scan', sets ranges within configurable
    angle intervals to NaN, and publishes the result to '/scan_filtered'.
    """

    def __init__(self):
        """Initialize the laser filter node with parameters and pub/sub."""
        super().__init__('laser_filter_node')

        # Declare parameter as a flat list
        self.declare_parameter('filter_ranges', [
            0.5566728, 0.9268571,
            2.2142123, 2.6901637,
            3.6900000, 4.1386122,
            5.3913217, 5.7438788
        ])

        # Convert flat list to (lower, upper) pairs
        filter_ranges_flat = (
            self.get_parameter('filter_ranges')
            .get_parameter_value().double_array_value
        )
        self.filter_ranges = [
            (filter_ranges_flat[i], filter_ranges_flat[i + 1])
            for i in range(0, len(filter_ranges_flat), 2)
        ]

        # Subscription that will receive unfiltered scans
        self.scan_sub = self.create_subscription(
            LaserScan, '/ldlidar_node/scan', self.scan_callback, 10)

        # Publisher for filtered scans
        self.scan_pub = self.create_publisher(LaserScan, '/scan_filtered', 10)

    def scan_callback(self, msg):
        """Receive a LaserScan, zero out filtered angle ranges, and republish.

        Args:
            msg: sensor_msgs.msg.LaserScan unfiltered message received.
        """
        # Copy list
        filtered_ranges = list(msg.ranges)

        # Initialize beginning and increment
        angle_min = msg.angle_min
        angle_increment = msg.angle_increment

        # Filter based on the ranges
        for i in range(len(filtered_ranges)):
            angle = angle_min + i * angle_increment

            for lower, upper in self.filter_ranges:
                if lower <= angle <= upper:
                    filtered_ranges[i] = float('nan')

        # Create filtered message
        filtered_msg = LaserScan()
        filtered_msg.header = msg.header
        filtered_msg.angle_min = msg.angle_min
        filtered_msg.angle_max = msg.angle_max
        filtered_msg.angle_increment = msg.angle_increment
        filtered_msg.time_increment = msg.time_increment
        filtered_msg.scan_time = msg.scan_time
        filtered_msg.range_min = msg.range_min
        filtered_msg.range_max = msg.range_max
        filtered_msg.ranges = filtered_ranges
        filtered_msg.intensities = msg.intensities

        # Publish
        self.scan_pub.publish(filtered_msg)


def main(args=None):
    """Create a LaserFilterNode and spin until shutdown."""
    rclpy.init(args=args)
    node = LaserFilterNode()

    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        node.get_logger().info('KeyboardInterrupt received. Shutting down...')
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()
