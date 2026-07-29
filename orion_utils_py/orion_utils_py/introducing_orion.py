#!/usr/bin/env python3
"""Execute an introductory gesture sequence for ORION demos."""

import time

import rclpy
from geometry_msgs.msg import TwistStamped
from rclpy.node import Node
from std_msgs.msg import Float64MultiArray, Int32


class GestureSequenceNode(Node):
    """Execute a scripted gesture sequence to introduce ORION."""

    def __init__(self):
        """Initialize arm, emotion, and cmd_vel publishers, then run the sequence."""
        super().__init__('gesture_sequence_node')

        self.arm_pub = self.create_publisher(
            Float64MultiArray, '/simple_left_arm_controller/commands', 10)
        self.emotion_pub = self.create_publisher(Int32, '/emotion/int', 10)
        self.cmd_vel_pub = self.create_publisher(
            TwistStamped, '/mobile_base_controller/cmd_vel', 10)

        time.sleep(1)

        self.run_sequence()

    def run_sequence(self):
        """Execute the full introduction gesture sequence and shut down."""
        self.publish_arm_position(0.0)

        self.publish_emotion(1)

        self.publish_cmd_vel_linear(0.2, duration=1.0)

        self.publish_emotion(3)

        self.publish_arm_position(-1.0)
        time.sleep(0.5)
        self.publish_arm_position(0.0)
        time.sleep(0.5)
        self.publish_arm_position(-1.0)
        time.sleep(0.5)
        self.publish_arm_position(0.0)

        self.get_logger().info('Gesture sequence completed. Shutting down...')
        rclpy.shutdown()

    def publish_arm_position(self, position):
        """Publish a single arm joint position command.

        Args:
            position: Target arm joint angle in radians.
        """
        msg = Float64MultiArray()
        msg.data = [position]
        self.arm_pub.publish(msg)
        self.get_logger().info(f'Published arm position: {position}')
        time.sleep(0.5)

    def publish_emotion(self, value):
        """Publish an emotion index to the emotion topic.

        Args:
            value: Integer emotion identifier to publish.
        """
        msg = Int32()
        msg.data = value
        self.emotion_pub.publish(msg)
        self.get_logger().info(f'Published emotion: {value}')
        time.sleep(0.5)

    def publish_cmd_vel_linear(self, linear_x, duration=1.0):
        """Publish a forward velocity command and then stop the robot.

        Args:
            linear_x: Forward linear velocity in m/s.
            duration: Time in seconds to hold the velocity before stopping.
        """
        twist = TwistStamped()
        twist.twist.linear.x = linear_x
        self.cmd_vel_pub.publish(twist)
        self.get_logger().info(f'Published forward velocity: {linear_x}')
        time.sleep(duration)

        # Stop the robot
        twist.twist.linear.x = 0.0
        self.cmd_vel_pub.publish(twist)
        self.get_logger().info('Published stop velocity')


def main(args=None):
    """Initialize the node and spin until shutdown."""
    rclpy.init(args=args)
    node = GestureSequenceNode()
    rclpy.spin(node)


if __name__ == '__main__':
    main()
