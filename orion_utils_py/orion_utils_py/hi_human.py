#!/usr/bin/env python3

import rclpy
from rclpy.node import Node
from geometry_msgs.msg import TwistStamped
from std_msgs.msg import Float64MultiArray, Int32
import time

class OrionSimpleSequence(Node):
    def __init__(self):
        super().__init__('orion_simple_sequence')

        # Publishers
        self.cmd_vel_pub = self.create_publisher(
            TwistStamped,
            '/mobile_base_controller/cmd_vel',
            10
        )

        self.left_arm_pub = self.create_publisher(
            Float64MultiArray,
            '/simple_left_arm_controller/commands',
            10
        )

        self.emotion_pub = self.create_publisher(
            Int32,
            '/emotion/int',
            10
        )

        self.get_logger().info('ORION simple sequence node started')

    def publish_emotion(self, emotion_id: int):
        msg = Int32()
        msg.data = emotion_id
        self.emotion_pub.publish(msg)
        self.get_logger().info(f'Emotion set to {emotion_id}')

    def move_forward(self, speed: float, duration: float, hz: float = 20.0):
        twist = TwistStamped()
        twist.twist.linear.x = speed
        twist.twist.angular.z = 0.0

        period = 1.0 / hz
        start_time = time.time()

        while time.time() - start_time < duration and rclpy.ok():
            twist.header.stamp = self.get_clock().now().to_msg()
            self.cmd_vel_pub.publish(twist)
            time.sleep(period)

        # Stop robot
        twist.twist.linear.x = 0.0
        self.cmd_vel_pub.publish(twist)

    def wave_left_arm(self, start: float = 0.0, end: float = 1.57):
        msg = Float64MultiArray()
        msg.data = [start]
        self.left_arm_pub.publish(msg)
        time.sleep(0.5)

        msg.data = [end]
        self.left_arm_pub.publish(msg)
        time.sleep(0.5)

        msg.data = [start]
        self.left_arm_pub.publish(msg)
        self.get_logger().info('Left arm greeting executed')

    def rotate_right(self, angular_speed: float, duration: float, hz: float = 20.0):
        twist = TwistStamped()
        twist.twist.linear.x = 0.0
        twist.twist.angular.z = -angular_speed  # negative = right

        period = 1.0 / hz
        start_time = time.time()

        while time.time() - start_time < duration and rclpy.ok():
            twist.header.stamp = self.get_clock().now().to_msg()
            self.cmd_vel_pub.publish(twist)
            time.sleep(period)

        # Stop rotation
        twist.twist.angular.z = 0.0
        self.cmd_vel_pub.publish(twist)


def main(args=None):
    rclpy.init(args=args)
    node = OrionSimpleSequence()

    # Small delay to ensure publishers are ready
    time.sleep(1.0)

    # 1. Set emotion
    node.publish_emotion(4)
    node.publish_emotion(4)
    time.sleep(0.5)

    # 2. Move forward for 1.5 seconds at 0.75 m/s
    node.move_forward(speed=1.5, duration=0.75, hz=20.0)

    # 3. Left arm greeting
    node.wave_left_arm(start=0.0, end=-1.57)
    node.wave_left_arm(start=0.0, end=-1.57)
    time.sleep(0.5)

    node.publish_emotion(7)
    node.publish_emotion(7)
    node.publish_emotion(7)
    time.sleep(0.5)

    # 4. Rotate right at 0.5 rad/s for 1.5 seconds
    node.rotate_right(angular_speed=2.5, duration=4.0, hz=20.0)

    node.get_logger().info('ORION sequence finished')

    node.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()
