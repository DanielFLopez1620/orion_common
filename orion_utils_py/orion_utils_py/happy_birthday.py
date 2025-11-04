import rclpy
from rclpy.node import Node
from std_msgs.msg import Int64, Float64MultiArray, String
import math
import time


class OrionCelebrator(Node):

    def __init__(self):
        super().__init__('orion_celebrator')

        # --- Publishers ---
        self.emotion_pub = self.create_publisher(Int64, '/emotion/int', 10)
        self.arm_right_pub = self.create_publisher(Float64MultiArray, '/simple_right_arm_controller/commands', 10)
        self.arm_left_pub = self.create_publisher(Float64MultiArray, '/simple_left_arm_controller/commands', 10)
        self.tts_pub = self.create_publisher(String, '/orion_response', 10)

        # --- Publish emotion once at start ---
        msg = Int64()
        msg.data = 1
        self.emotion_pub.publish(msg)
        self.get_logger().info('Emotion published: 1')

        # --- Timer for arm waving ---
        self.start_time = time.time()
        self.create_timer(0.1, self.arm_motion_callback)  # 10 Hz

        # --- Publish creative birthday message ---
        bday_msg = String()
        bday_msg.data = "¡Feliz cumpleaños! Que tus circuitos brillen más que mis LEDs, y que tengas un año lleno de aventuras épicas."
        self.tts_pub.publish(bday_msg)
        self.get_logger().info(f'TTS message published: {bday_msg.data}')

    def arm_motion_callback(self):
        """Make arms oscillate between -1 and 1 rad (sine wave)."""
        elapsed = time.time() - self.start_time
        angle = math.sin(elapsed * 2.0)

        # Right arm
        msg_right = Float64MultiArray()
        msg_right.data = [angle]
        self.arm_right_pub.publish(msg_right)

        # Left arm
        msg_left = Float64MultiArray()
        msg_left.data = [-angle]
        self.arm_left_pub.publish(msg_left)


def main(args=None):
    rclpy.init(args=args)
    node = OrionCelebrator()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()
