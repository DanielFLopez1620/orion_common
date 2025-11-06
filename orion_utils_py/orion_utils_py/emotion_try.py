#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from std_msgs.msg import Int32, Float64MultiArray
import math
import time

class OrionEmotionTest(Node):
    def __init__(self):
        super().__init__('orion_emotion_test')

        # Publishers
        self.pub_emotion = self.create_publisher(Int32, '/emotion/int', 10)
        self.pub_left_arm = self.create_publisher(Float64MultiArray, '/simple_left_arm_controller/commands', 10)
        self.pub_right_arm = self.create_publisher(Float64MultiArray, '/simple_right_arm_controller/commands', 10)

        # Emotion control
        self.current_emotion = 0
        self.max_emotion = 7
        self.last_emotion_time = time.time()
        self.emotion_interval = 2.0  # seconds

        # Arm control
        self.last_arm_time = time.time()
        self.arm_period = 5.0  # seconds
        self.arm_amplitude = 1.0  # radians

        # Main timer (50 Hz)
        self.timer = self.create_timer(0.02, self.update_loop)
        self.get_logger().info("✅ OrionEmotionTest node started successfully!")

    def update_loop(self):
        now = time.time()

        # --- EMOTION CYCLING ---
        if now - self.last_emotion_time >= self.emotion_interval:
            self.current_emotion = (self.current_emotion + 1) % (self.max_emotion + 1)
            msg = Int32()
            msg.data = self.current_emotion
            self.pub_emotion.publish(msg)
            self.get_logger().info(f"[EMOTION] Changed to {self.current_emotion}")
            self.last_emotion_time = now

        # --- ARM MOTION (sine wave) ---
        elapsed = (now - self.last_arm_time) % self.arm_period
        phase = 2.0 * math.pi * elapsed / self.arm_period
        angle = self.arm_amplitude * math.sin(phase)

        msg_left = Float64MultiArray()
        msg_right = Float64MultiArray()
        msg_left.data = [angle]
        msg_right.data = [angle]

        self.pub_left_arm.publish(msg_left)
        self.pub_right_arm.publish(msg_right)

def main(args=None):
    rclpy.init(args=args)
    node = OrionEmotionTest()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        node.get_logger().info("🛑 Stopping OrionEmotionTest node...")
    finally:
        node.destroy_node()
        rclpy.shutdown()

if __name__ == '__main__':
    main()
