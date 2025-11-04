import rclpy
from rclpy.node import Node
from geometry_msgs.msg import TwistStamped
from nav_msgs.msg import Odometry
import time

THRESHOLD = 0.02

class LatencyMeasure(Node):
    def __init__(self):
        super().__init__('latency_measure_node')
        self.cmd_vel_time = None
        self.measured = False
        self.create_subscription(TwistStamped, '/mobile_base_controller/cmd_vel', self.cmd_callback, 10)
        self.create_subscription(Odometry, '/mobile_base_controller/odom', self.odom_callback, 10)

    def cmd_callback(self, msg):
        self.cmd_vel_time = self.get_clock().now().nanoseconds / 1e9
        self.get_logger().info(f'[CMD_VEL] Sent to {self.cmd_vel_time:.4f}s')

    def odom_callback(self, msg):
        if not self.measured and self.cmd_vel_time:
            vx = msg.twist.twist.linear.x
            if abs(vx) > THRESHOLD:
                move_time = self.get_clock().now().nanoseconds / 1e9
                latency = move_time - self.cmd_vel_time
                self.get_logger().info(f'[ODOM] detected move {move_time:.4f}s')
                self.get_logger().info(f'Total latency: {latency:.4f}s')
                self.measured = True

def main(args=None):
    rclpy.init(args=args)
    node = LatencyMeasure()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()
