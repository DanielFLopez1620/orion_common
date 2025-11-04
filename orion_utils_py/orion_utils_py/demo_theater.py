import rclpy
from rclpy.node import Node
from geometry_msgs.msg import TwistStamped
from std_msgs.msg import String, Int32
import time

class RobotPerformer(Node):
    """
    A node to execute a sequence of movements and broadcast messages
    for a robot performance.
    """
    def __init__(self):
        super().__init__('robot_performer_node')

        self.cmd_vel_pub = self.create_publisher(
            TwistStamped,
            '/mobile_base_controller/cmd_vel',
            10
        )
        self.response_pub = self.create_publisher(
            String,
            '/orion_response',
            10
        )
        self.emotion_pub = self.create_publisher(
            Int32,
            '/emotion/int',
            10
        )

        self.get_logger().info('Robot Performer Node Initialized.')

    def create_twist_stamped(self, linear_x, angular_z):
        """Helper function to create a populated TwistStamped message."""
        twist = TwistStamped()
        twist.header.stamp = self.get_clock().now().to_msg()
        twist.header.frame_id = 'base_link'
        twist.twist.linear.x = linear_x
        twist.twist.angular.z = angular_z
        return twist

    def perform_sequence(self):
        """Executes the full movement and messaging sequence."""
        self.get_logger().info('Starting performance sequence...')

        # Define speeds
        FORWARD_SPEED = 2.0  # m/s
        ANGULAR_SPEED = -2.0 # rad/s (negative for right turn)
        SPIN_SPEED = 1.5     # rad/s (positive for left spin)

        self.get_logger().info('1) Moving forward for 2.5 seconds.')
        forward_cmd = self.create_twist_stamped(FORWARD_SPEED, 0.0)
        start_time = self.get_clock().now()
        duration = rclpy.duration.Duration(seconds=5.0)

        while (self.get_clock().now() - start_time) < duration:
            self.cmd_vel_pub.publish(forward_cmd)
            time.sleep(0.05)

        self.cmd_vel_pub.publish(self.create_twist_stamped(0.0, 0.0))
        self.get_logger().info('Movement 1 completed (Forward).')
        self.get_logger().info('2) Turning right for 0.5 seconds.')
        right_turn_cmd = self.create_twist_stamped(0.0, ANGULAR_SPEED)
        start_time = self.get_clock().now()
        duration = rclpy.duration.Duration(seconds=1.5)

        while (self.get_clock().now() - start_time) < duration:
            self.cmd_vel_pub.publish(right_turn_cmd)
            time.sleep(0.05)

        self.cmd_vel_pub.publish(self.create_twist_stamped(0.0, 0.0))
        self.get_logger().info('Movement 2 completed (Right Turn).')
        time.sleep(0.5)

        self.get_logger().info('3) Publishing final response message.')
        msg = String()
        msg.data = "Buenas noches a todos. Muchas gracias por venir. Esperamos hayan disfrutado la demo!"
        self.response_pub.publish(msg)
        self.get_logger().info(f'Response published: "{msg.data}"')

        self.get_logger().info('4) Publishing emotion 7.')
        emotion_7 = Int32()
        emotion_7.data = 7
        self.emotion_pub.publish(emotion_7)
        self.get_logger().info('Emotion 7 published.')

        self.get_logger().info('5) Waiting 2 seconds...')
        time.sleep(2.0)

        self.get_logger().info('5) Publishing emotion 3.')
        emotion_3 = Int32()
        emotion_3.data = 3
        self.emotion_pub.publish(emotion_3)
        self.get_logger().info('Emotion 3 published.')
        time.sleep(0.5)

        self.get_logger().info('6) Spinning for 1 second.')
        spin_cmd = self.create_twist_stamped(0.0, SPIN_SPEED)
        start_time = self.get_clock().now()
        duration = rclpy.duration.Duration(seconds=1.0)

        while (self.get_clock().now() - start_time) < duration:
            self.cmd_vel_pub.publish(spin_cmd)
            time.sleep(0.05)

        self.cmd_vel_pub.publish(self.create_twist_stamped(0.0, 0.0))
        self.get_logger().info('Movement 6 completed (Spin).')

        self.get_logger().info('Sequence finished. Shutting down.')


def main(args=None):
    rclpy.init(args=args)
    performer = RobotPerformer()

    performer.perform_sequence()

    performer.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()