#!/usr/bin/env python3
"""MPU6050 IMU driver node for ORION G Mov module.

Reads accelerometer, gyroscope, and die temperature from the MPU6050 over I2C
and publishes them as sensor_msgs/Imu and sensor_msgs/Temperature.

Calibration
-----------
Run ``ros2 run orion_utils_py mpu6050_calibration`` once with the robot still
and level.  The tool writes bias offsets to ~/.ros/mpu6050_calibration.yaml.
This node loads that file at startup (calibration_file parameter) and subtracts
the stored offsets from every reading.  If the file is absent, zero biases are
used and a warning is logged.

Published topics:
  /g_mov/imu/raw         sensor_msgs/Imu
  /g_mov/imu/temperature sensor_msgs/Temperature
"""

import math
import os

import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Imu, Temperature

import smbus2

_REG_PWR_MGMT_1   = 0x6B
_REG_ACCEL_XOUT_H = 0x3B
_REG_TEMP_OUT_H   = 0x41
_REG_GYRO_XOUT_H  = 0x43

_ACCEL_SCALE = 16384.0    # LSB/g   (±2g default range)
_GYRO_SCALE  = 131.0      # LSB/°/s (±250°/s default range)
_G           = 9.80665    # m/s²
_ACCEL_VAR   = 0.04 ** 2  # (m/s²)² — typical MPU6050 noise
_GYRO_VAR    = 0.003 ** 2  # (rad/s)²

_DEFAULT_CAL = os.path.expanduser('~/.ros/mpu6050_calibration.yaml')


def _load_calibration(path: str, logger) -> dict:
    """Parse the YAML calibration file and return a bias dict.

    Returns zero biases when the file is missing or malformed so the node can
    still publish uncalibrated data.
    """
    zeros = {'ax': 0.0, 'ay': 0.0, 'az': 0.0,
             'gx': 0.0, 'gy': 0.0, 'gz': 0.0}

    if not os.path.isfile(path):
        logger.warn(
            f'Calibration file not found: {path}  '
            'Using zero biases. Run mpu6050_calibration to generate it.'
        )
        return zeros

    try:
        # Parse minimal YAML manually to avoid adding a PyYAML dependency.
        # Expected format (written by mpu6050_calibration.py):
        #   accel_bias:
        #     x: <float>
        #     y: <float>
        #     z: <float>
        #   gyro_bias:
        #     x: <float>
        #     ...
        section = None
        values  = {}
        with open(path) as f:
            for line in f:
                line = line.strip()
                if not line or line.startswith('#'):
                    continue
                if line == 'accel_bias:':
                    section = 'accel'
                elif line == 'gyro_bias:':
                    section = 'gyro'
                elif ':' in line and section:
                    key, _, val = line.partition(':')
                    values[f'{section}_{key.strip()}'] = float(val.strip())

        bias = {
            'ax': values.get('accel_x', 0.0),
            'ay': values.get('accel_y', 0.0),
            'az': values.get('accel_z', 0.0),
            'gx': values.get('gyro_x',  0.0),
            'gy': values.get('gyro_y',  0.0),
            'gz': values.get('gyro_z',  0.0),
        }
        logger.info(
            f'Calibration loaded from {path}  '
            f'accel=({bias["ax"]:+.4f}, {bias["ay"]:+.4f}, {bias["az"]:+.4f}) m/s²  '
            f'gyro=({bias["gx"]:+.4f}, {bias["gy"]:+.4f}, {bias["gz"]:+.4f}) rad/s'
        )
        return bias

    except Exception as exc:
        logger.error(f'Failed to parse calibration file {path}: {exc}  Using zero biases.')
        return zeros


class MPU6050Node(Node):
    """Read MPU6050 via I2C and publish Imu + Temperature at a fixed rate."""

    def __init__(self):
        """Initialize I2C bus, wake up the MPU6050, load calibration, and create publishers."""
        super().__init__('mpu6050_node')

        self.declare_parameter('i2c_bus',          1)
        self.declare_parameter('i2c_address',      0x68)
        self.declare_parameter('publish_rate',     20.0)
        self.declare_parameter('frame_id',         'g_mov_imu')
        self.declare_parameter('calibration_file', _DEFAULT_CAL)

        bus_id      = self.get_parameter('i2c_bus').value
        self._addr  = self.get_parameter('i2c_address').value
        rate        = self.get_parameter('publish_rate').value
        self._frame = self.get_parameter('frame_id').value
        cal_file    = self.get_parameter('calibration_file').value

        self._bias = _load_calibration(cal_file, self.get_logger())

        self._bus = smbus2.SMBus(bus_id)
        self._bus.write_byte_data(self._addr, _REG_PWR_MGMT_1, 0x00)

        self._pub_imu  = self.create_publisher(Imu,         '/g_mov/imu/raw',         10)
        self._pub_temp = self.create_publisher(Temperature, '/g_mov/imu/temperature', 10)

        self.create_timer(1.0 / rate, self._read_and_publish)

    def _read_raw(self, reg: int) -> int:
        """Read two bytes from reg and return as a signed 16-bit integer."""
        data = self._bus.read_i2c_block_data(self._addr, reg, 2)
        raw  = (data[0] << 8) | data[1]
        return raw - 65536 if raw > 32767 else raw

    def _read_and_publish(self):
        """Sample sensor registers, apply calibration, and publish Imu + Temperature."""
        now = self.get_clock().now().to_msg()

        ax = self._read_raw(_REG_ACCEL_XOUT_H)     * _G / _ACCEL_SCALE - self._bias['ax']
        ay = self._read_raw(_REG_ACCEL_XOUT_H + 2) * _G / _ACCEL_SCALE - self._bias['ay']
        az = self._read_raw(_REG_ACCEL_XOUT_H + 4) * _G / _ACCEL_SCALE - self._bias['az']

        gx = self._read_raw(_REG_GYRO_XOUT_H)     * math.pi / (180.0 * _GYRO_SCALE) - self._bias['gx']
        gy = self._read_raw(_REG_GYRO_XOUT_H + 2) * math.pi / (180.0 * _GYRO_SCALE) - self._bias['gy']
        gz = self._read_raw(_REG_GYRO_XOUT_H + 4) * math.pi / (180.0 * _GYRO_SCALE) - self._bias['gz']

        t_raw  = self._read_raw(_REG_TEMP_OUT_H)
        temp_c = t_raw / 340.0 + 36.53

        imu = Imu()
        imu.header.stamp    = now
        imu.header.frame_id = self._frame
        # Orientation is not computed — only 6 DOF (no magnetometer).
        # ROS convention: set covariance[0] = -1 to signal unavailability.
        imu.orientation_covariance[0] = -1.0

        imu.linear_acceleration.x = ax
        imu.linear_acceleration.y = ay
        imu.linear_acceleration.z = az
        imu.linear_acceleration_covariance = [
            _ACCEL_VAR, 0.0, 0.0,
            0.0, _ACCEL_VAR, 0.0,
            0.0, 0.0, _ACCEL_VAR,
        ]

        imu.angular_velocity.x = gx
        imu.angular_velocity.y = gy
        imu.angular_velocity.z = gz
        imu.angular_velocity_covariance = [
            _GYRO_VAR, 0.0, 0.0,
            0.0, _GYRO_VAR, 0.0,
            0.0, 0.0, _GYRO_VAR,
        ]
        self._pub_imu.publish(imu)

        temp = Temperature()
        temp.header.stamp    = now
        temp.header.frame_id = self._frame
        temp.temperature     = temp_c
        temp.variance        = 0.0
        self._pub_temp.publish(temp)

    def destroy_node(self):
        """Close the I2C bus before shutdown."""
        self._bus.close()
        super().destroy_node()


def main(args=None):
    """Entry point for the mpu6050_node executable."""
    rclpy.init(args=args)
    node = MPU6050Node()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()
