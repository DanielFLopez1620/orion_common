#!/usr/bin/env python3
"""Measure cmd_vel-to-odometry latency for ORION mobile base diagnostics."""

import rclpy
from geometry_msgs.msg import TwistStamped
from nav_msgs.msg import Odometry
from rclpy.node import Node

THRESHOLD = 0.02  # m/s minimum linear velocity to detect movement


class LatencyMeasure(Node):
    """Measure latency between a velocity command and the first observed movement."""

    def __init__(self):
        """Subscribe to cmd_vel and odom to record timestamps for latency calculation."""
        super().__init__('latency_measure_node')
        self.cmd_vel_time = None
        self.measured = False
        self.create_subscription(
            TwistStamped, '/mobile_base_controller/cmd_vel',
            self.cmd_callback, 10)
        self.create_subscription(
            Odometry, '/mobile_base_controller/odom',
            self.odom_callback, 10)

    def cmd_callback(self, msg):
        """Record the timestamp when a velocity command is observed.

        Args:
            msg: geometry_msgs.msg.TwistStamped velocity command message.
        """
        self.cmd_vel_time = self.get_clock().now().nanoseconds / 1e9
        self.get_logger().info(f'[CMD_VEL] Sent to {self.cmd_vel_time:.4f}s')

    def odom_callback(self, msg):
        """Compute latency when movement above threshold is first detected.

        Args:
            msg: nav_msgs.msg.Odometry odometry message.
        """
        if not self.measured and self.cmd_vel_time:
            vx = msg.twist.twist.linear.x
            if abs(vx) > THRESHOLD:
                move_time = self.get_clock().now().nanoseconds / 1e9
                latency = move_time - self.cmd_vel_time
                self.get_logger().info(f'[ODOM] detected move {move_time:.4f}s')
                self.get_logger().info(f'Total latency: {latency:.4f}s')
                self.measured = True


def main(args=None):
    """Spin LatencyMeasure node until shutdown."""
    rclpy.init(args=args)
    node = LatencyMeasure()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()
