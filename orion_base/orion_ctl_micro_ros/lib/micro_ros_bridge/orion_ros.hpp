/**
 * @file orion_ros.hpp
 * @brief Micro-ROS bridge interface for ORION differential drive control.
 *
 * This header defines the PUBLIC interface to micro-ROS functionality.
 * All micro-ROS infrastructure (rclc support, nodes, publishers, subscribers, executor)
 * is ENCAPSULATED in orion_ros.cpp using an anonymous namespace (private to that file).
 */

#ifndef ORION_ROS_HPP
#define ORION_ROS_HPP

#include <stdint.h>

/**
 * @defgroup OrionROS Micro-ROS Bridge Functions
 * @brief Interface to micro-ROS for ORION control firmware.
 *
 * All ROS infrastructure (rclc_support_t, rcl_node_t, publishers, subscribers, executor)
 * lives in an anonymous namespace in orion_ros.cpp. These functions provide the ONLY
 * public access point.
 * @{
 */

/**
 * @brief Initializes micro-ROS: support, node, publishers, subscribers, executor.
 *
 * Sets up serial transport, creates node, and initializes all publishers/subscribers.
 * Must be called ONCE during setup(). Blocks until connection established or timeout.
 */
void orion_micro_ros_init();

/**
 * @brief Publishes encoder feedback to ROS.
 *
 * @param left_count Encoder count from left motor
 * @param right_count Encoder count from right motor
 *
 * Topics: /diff_ctl_left_enc (Int64), /diff_ctl_right_enc (Int64)
 */
void orion_micro_ros_publish_encoders(int64_t left_count, int64_t right_count);

/**
 * @brief Publishes servo position feedback to ROS.
 *
 * @param left_rad Current position of left servo (radians)
 * @param right_rad Current position of right servo (radians)
 *
 * Topics: /fwd_servo_left_feedback (Float32), /fwd_servo_right_feedback (Float32)
 */
void orion_micro_ros_publish_servo_feedback(float left_rad, float right_rad);

/**
 * @brief Processes pending ROS messages (callbacks, timer handlers).
 *
 * Spins the executor for the specified timeout. Should be called once per loop().
 * Non-blocking; returns after timeout_ms or when no more messages.
 *
 * @param timeout_ms Timeout in milliseconds
 */
void orion_micro_ros_spin(uint32_t timeout_ms);

/**
 * @brief Registers callback for motor speed commands.
 *
 * When a message arrives on /diff_ctl_motor_cmd, the provided callback is invoked
 * with (left_speed, right_speed).
 *
 * @param callback Function pointer: void callback(int left_speed, int right_speed)
 *
 * Topic: /diff_ctl_motor_cmd (Int64MultiArray with 2 elements)
 */
void orion_micro_ros_set_motor_cmd_callback(void (*callback)(int, int));

/**
 * @brief Registers callback for left servo position commands.
 *
 * @param callback Function pointer: void callback(float radians)
 *
 * Topic: /fwd_servo_left_cmd (Float32)
 */
void orion_micro_ros_set_servo_left_cmd_callback(void (*callback)(float));

/**
 * @brief Registers callback for right servo position commands.
 *
 * @param callback Function pointer: void callback(float radians)
 *
 * Topic: /fwd_servo_right_cmd (Float32)
 */
void orion_micro_ros_set_servo_right_cmd_callback(void (*callback)(float));

/** @} */

#endif
