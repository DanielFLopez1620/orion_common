#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from sensor_msgs.msg import LaserScan

# ----------------------- CLASS DEFINITIONS -----------------------------------

class LaserFilterNode(Node):
    """
    Class oriented to create a node that can filter pairs of ranges of lassers
    that come from a /scan topic of a LIDAR.

    Attributes
    ---
    filter_ranges : Float
        Array with the ranges to filter

    scan_sub : rclpy subscriber
        Subscriber of LaserScan messages

    scan_pub : rclpy publisher
        Publisher of LaserScan messages    

    Methods
    ---
    scan_callback :
        When a /scan message is received, filter the range recieved.

    """
    def __init__(self):
        """
        User defined constructor that initialize the laser_filter_node, prepare
        the params and start the subscriber (unfiltered) and publisher (filtered)
        """
        super().__init__('laser_filter_node')

        # Declare parameter as a flat list
        self.declare_parameter('filter_ranges', [
            0.5585054, 0.9250245,
            2.2165685, 2.6878075,
            3.7000975, 4.1364305,
            5.393067, 5.7421335
        ])

        # Convert arrays to ranges
        filter_ranges_flat = self.get_parameter('filter_ranges').get_parameter_value().double_array_value
        self.filter_ranges = [(filter_ranges_flat[i], filter_ranges_flat[i+1]) for i in range(0, len(filter_ranges_flat), 2)]

        # Suscription that will receive unfiltered scans
        self.scan_sub = self.create_subscription(LaserScan, '/ldlidar_node/scan', self.scan_callback, 10)

        # Publisher for filtered scans
        self.scan_pub = self.create_publisher(LaserScan, '/scan_filtered', 10)

    def scan_callback(self, msg):
        """
        Subscriber callback that receives Laser Scan meessages, filter the
        desired ranges and then proceed to send a filtered LaserScan mesage

        Params
        ---
        msg : sensor_msgs.msg.LaserScan
            Unfiltered message received
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

        # Create filtered messsage
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

# -------------------------- MAIN IMPLEMENTATION ------------------------------

def main(args=None):
    """
    Create a Laser Filter node and spin until shutdown or Keyboard interrupt
    """

    # Intiialize node
    rclpy.init(args=args)
    node = LaserFilterNode()

    try:
        # Spin under exception management
        rclpy.spin(node)
    except KeyboardInterrupt:
        # If Ctrl+C is called, interrupt
        node.get_logger().info("KeyboardInterrupt received. Shutting down...")
    finally:
        node.destroy_node()
        rclpy.shutdown()

if __name__ == '__main__':
    main()
