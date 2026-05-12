#!/usr/bin/env python3
"""MPU6050 IMU driver node for ORION G Mov module."""

import math

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


class MPU6050Node(Node):
    """Read MPU6050 via I2C and publish Imu + Temperature at a fixed rate."""

    def __init__(self):
        """Initialize I2C bus, wake up the MPU6050, and create publishers."""
        super().__init__('mpu6050_node')

        self.declare_parameter('i2c_bus',      1)
        self.declare_parameter('i2c_address',  0x68)
        self.declare_parameter('publish_rate', 20.0)
        self.declare_parameter('frame_id',     'g_mov_imu')

        bus_id       = self.get_parameter('i2c_bus').value
        self._addr   = self.get_parameter('i2c_address').value
        rate         = self.get_parameter('publish_rate').value
        self._frame  = self.get_parameter('frame_id').value

        self._bus = smbus2.SMBus(bus_id)
        self._bus.write_byte_data(self._addr, _REG_PWR_MGMT_1, 0x00)

        self._pub_imu  = self.create_publisher(Imu,         '/g_mov/imu/raw',         10)
        self._pub_temp = self.create_publisher(Temperature, '/g_mov/imu/temperature', 10)

        self.create_timer(1.0 / rate, self._read_and_publish)

    def _read_raw(self, reg):
        """Read two bytes from reg and return as a signed 16-bit integer."""
        data = self._bus.read_i2c_block_data(self._addr, reg, 2)
        raw  = (data[0] << 8) | data[1]
        return raw - 65536 if raw > 32767 else raw

    def _read_and_publish(self):
        """Sample sensor registers and publish Imu and Temperature messages."""
        now = self.get_clock().now().to_msg()

        ax = self._read_raw(_REG_ACCEL_XOUT_H)     * _G / _ACCEL_SCALE
        ay = self._read_raw(_REG_ACCEL_XOUT_H + 2) * _G / _ACCEL_SCALE
        az = self._read_raw(_REG_ACCEL_XOUT_H + 4) * _G / _ACCEL_SCALE

        gx = self._read_raw(_REG_GYRO_XOUT_H)     * math.pi / (180.0 * _GYRO_SCALE)
        gy = self._read_raw(_REG_GYRO_XOUT_H + 2) * math.pi / (180.0 * _GYRO_SCALE)
        gz = self._read_raw(_REG_GYRO_XOUT_H + 4) * math.pi / (180.0 * _GYRO_SCALE)

        t_raw  = self._read_raw(_REG_TEMP_OUT_H)
        temp_c = t_raw / 340.0 + 36.53  # MPU6050 datasheet conversion

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
