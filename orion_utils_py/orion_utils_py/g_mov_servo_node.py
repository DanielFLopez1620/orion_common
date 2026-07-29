#!/usr/bin/env python3
"""MG996R servo driver node for ORION G Mov pan/tilt module.

Bridges the ForwardOrion hardware interface to the physical servo
on GPIO12 via the kernel hardware-PWM sysfs interface (pwmchip0/pwm0).
No daemon required; timing is handled by the PWM hardware peripheral.

Topic flow (ros2_control active):
  ForwardOrion --> [fwd_servo_g_mov_cmd,      Float32] --> this node --> /sys/class/pwm/pwmchip0/pwm0/
  ForwardOrion <-- [fwd_servo_g_mov_feedback, Float32] <-- this node
"""

import math
import time
from pathlib import Path

import rclpy
from rclpy.node import Node
from std_msgs.msg import Float32

_JOINT_LOWER = -math.pi / 4
_JOINT_UPPER =  math.pi / 6

_PWM_ROOT  = Path('/sys/class/pwm/pwmchip0')
_PERIOD_NS = 20_000_000


class GMovServoNode(Node):
    """Drive G Mov MG996R via kernel hardware PWM and bridge ForwardOrion topics."""

    def __init__(self):
        """Export PWM channel, initialize servo to neutral, and create topics."""
        super().__init__('g_mov_servo_node')

        self.declare_parameter('pwm_channel',       0)
        self.declare_parameter('neutral_pulse_us',  1500)
        self.declare_parameter('min_pulse_us',      500)
        self.declare_parameter('max_pulse_us',      2500)
        self.declare_parameter('publish_rate',      20.0)
        self.declare_parameter('joint_lower',       _JOINT_LOWER)
        self.declare_parameter('joint_upper',       _JOINT_UPPER)

        channel       = self.get_parameter('pwm_channel').value
        self._neutral = self.get_parameter('neutral_pulse_us').value
        self._min_pw  = self.get_parameter('min_pulse_us').value
        self._max_pw  = self.get_parameter('max_pulse_us').value
        self._lower   = self.get_parameter('joint_lower').value
        self._upper   = self.get_parameter('joint_upper').value
        rate          = self.get_parameter('publish_rate').value

        self._pwm_dir = _PWM_ROOT / f'pwm{channel}'
        self._setup_pwm(channel)

        self._current_pos   = 0.0
        self._current_pulse = self._neutral

        self._pub_feedback = self.create_publisher(
            Float32, 'fwd_servo_g_mov_feedback', 10)
        self.create_subscription(
            Float32, 'fwd_servo_g_mov_cmd', self._on_cmd, 10)
        self.create_timer(1.0 / rate, self._publish_feedback)

    def _setup_pwm(self, channel: int):
        if not self._pwm_dir.exists():
            try:
                (_PWM_ROOT / 'export').write_text(str(channel))
            except PermissionError:
                raise PermissionError(
                    f'Cannot export PWM channel {channel}: '
                    f'{_PWM_ROOT}/export is not writable. '
                    'Run as root or install pwm-setup.service first:\n'
                    f'  echo {channel} | sudo tee {_PWM_ROOT}/export\n'
                    f'  sudo chmod -R a+rw {_PWM_ROOT}/'
                ) from None
            # Wait for the kernel to create the sysfs entries
            deadline = time.monotonic() + 1.0
            while not self._pwm_dir.exists():
                if time.monotonic() > deadline:
                    raise RuntimeError(
                        f'Timed out waiting for {self._pwm_dir} after export')
                time.sleep(0.01)

        # Period must be written before duty_cycle
        (self._pwm_dir / 'period').write_text(str(_PERIOD_NS))
        (self._pwm_dir / 'duty_cycle').write_text(str(self._neutral * 1000))
        (self._pwm_dir / 'enable').write_text('1')

    def _on_cmd(self, msg: Float32):
        """Receive position command, clamp to joint limits, and drive the servo.

        duty_cycle is only written when the computed pulse width changes to avoid
        unnecessary sysfs traffic and prevent the servo from hunting.
        """
        pos   = max(self._lower, min(self._upper, float(msg.data)))
        pulse = self._neutral + (pos / (math.pi / 2.0)) * (self._max_pw - self._neutral)
        pulse = int(max(self._min_pw, min(self._max_pw, pulse)))
        if pulse != self._current_pulse:
            (self._pwm_dir / 'duty_cycle').write_text(str(pulse * 1000))
            self._current_pulse = pulse
        self._current_pos = pos

    def _publish_feedback(self):
        """Publish current servo position as feedback for ForwardOrion."""
        msg      = Float32()
        msg.data = float(self._current_pos)
        self._pub_feedback.publish(msg)

    def destroy_node(self):
        """Zero duty cycle and disable PWM output on shutdown."""
        (self._pwm_dir / 'duty_cycle').write_text('0')
        (self._pwm_dir / 'enable').write_text('0')
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
