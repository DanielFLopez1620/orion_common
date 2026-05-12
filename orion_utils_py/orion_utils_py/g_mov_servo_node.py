#!/usr/bin/env python3
"""MG996R servo driver node for ORION G Mov pan/tilt module.

Bridges the ForwardOrion hardware interface to the physical servo
on GPIO16 via pigpiod.

Topic flow (ros2_control active):
  ForwardOrion --> [fwd_servo_g_mov_cmd,      Float32] --> this node --> GPIO16 PWM
  ForwardOrion <-- [fwd_servo_g_mov_feedback, Float32] <-- this node
"""

import math

import pigpio
import rclpy
from rclpy.node import Node
from std_msgs.msg import Float32

_JOINT_LOWER = -math.pi / 4   # -45° (from URDF g_mov_servo_conn_joint limit)
_JOINT_UPPER =  math.pi / 6   # +30° (from URDF g_mov_servo_conn_joint limit)


class GMovServoNode(Node):
    """Drive G Mov MG996R via pigpio and bridge ForwardOrion feedback/command topics."""

    def __init__(self):
        """Connect to pigpiod, initialize servo to neutral, and create topics."""
        super().__init__('g_mov_servo_node')

        self.declare_parameter('gpio_pin',          16)
        self.declare_parameter('neutral_pulse_us',  1500)
        self.declare_parameter('min_pulse_us',      500)
        self.declare_parameter('max_pulse_us',      2500)
        self.declare_parameter('publish_rate',      20.0)
        self.declare_parameter('joint_lower',       _JOINT_LOWER)
        self.declare_parameter('joint_upper',       _JOINT_UPPER)

        self._gpio    = self.get_parameter('gpio_pin').value
        self._neutral = self.get_parameter('neutral_pulse_us').value
        self._min_pw  = self.get_parameter('min_pulse_us').value
        self._max_pw  = self.get_parameter('max_pulse_us').value
        self._lower   = self.get_parameter('joint_lower').value
        self._upper   = self.get_parameter('joint_upper').value
        rate          = self.get_parameter('publish_rate').value

        self._pi = pigpio.pi()
        if not self._pi.connected:
            self.get_logger().error('Cannot connect to pigpiod — is the daemon running?')
            raise RuntimeError('pigpiod unavailable')

        self._current_pos = 0.0
        self._pi.set_servo_pulsewidth(self._gpio, self._neutral)

        # ForwardOrion bridge topics — message type Float32, position in radians
        self._pub_feedback = self.create_publisher(
            Float32, 'fwd_servo_g_mov_feedback', 10)
        self.create_subscription(
            Float32, 'fwd_servo_g_mov_cmd', self._on_cmd, 10)

        self.create_timer(1.0 / rate, self._publish_feedback)

    def _on_cmd(self, msg: Float32):
        """Receive position command, clamp to joint limits, and drive the servo."""
        pos   = max(self._lower, min(self._upper, float(msg.data)))
        pulse = self._neutral + (pos / (math.pi / 2.0)) * (self._max_pw - self._neutral)
        pulse = int(max(self._min_pw, min(self._max_pw, pulse)))
        self._pi.set_servo_pulsewidth(self._gpio, pulse)
        self._current_pos = pos

    def _publish_feedback(self):
        """Publish current servo position as feedback for ForwardOrion."""
        msg      = Float32()
        msg.data = float(self._current_pos)
        self._pub_feedback.publish(msg)

    def destroy_node(self):
        """Release PWM signal and disconnect pigpio on shutdown."""
        self._pi.set_servo_pulsewidth(self._gpio, 0)
        self._pi.stop()
        super().destroy_node()


def main(args=None):
    """Entry point for the g_mov_servo_node executable."""
    rclpy.init(args=args)
    node = GMovServoNode()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()
