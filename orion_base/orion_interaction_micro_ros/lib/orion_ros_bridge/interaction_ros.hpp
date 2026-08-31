/**
 * @file interaction_ros.hpp
 * @brief Micro-ROS bridge interface for ORION interaction module.
 *
 * This header defines the PUBLIC interface to micro-ROS functionality.
 * All micro-ROS infrastructure (rclc support, node, publishers, subscriber, executor)
 * is ENCAPSULATED in interaction_ros.cpp using an anonymous namespace (private to
 * that file).
 */

#ifndef INTERACTION_ROS_HPP
#define INTERACTION_ROS_HPP

#include <stdint.h>

/**
 * @defgroup InteractionROS Micro-ROS Bridge Functions
 * @brief Interface to micro-ROS for ORION interaction firmware.
 *
 * All ROS infrastructure (rclc_support_t, rcl_node_t, publishers, subscriber, executor)
 * lives in an anonymous namespace in interaction_ros.cpp. These functions provide the
 * ONLY public access point.
 * @{
 */

/**
 * @brief Initializes micro-ROS: support, node, publishers, subscriber, executor.
 *
 * Sets up serial transport, creates node, and initializes all publishers/subscriber.
 * Must be called ONCE during setup(). Blocks until connection established or timeout.
 */
void interaction_micro_ros_init();

/**
 * @brief Publishes the four touch sensor states to ROS.
 *
 * @param ur Upper-right touch sensor state
 * @param ul Upper-left touch sensor state
 * @param lr Lower-right touch sensor state
 * @param ll Lower-left touch sensor state
 *
 * Topics: /interaction/touch_ur, /interaction/touch_ul,
 *         /interaction/touch_lr, /interaction/touch_ll (std_msgs/msg/Bool)
 */
void interaction_micro_ros_publish_touch(bool ur, bool ul, bool lr, bool ll);

/**
 * @brief Publishes a heartbeat pulse to ROS (connectivity monitoring).
 *
 * Topic: /interaction/heartbeat (std_msgs/msg/Bool)
 */
void interaction_micro_ros_publish_heartbeat();

/**
 * @brief Processes pending ROS messages (callbacks, timer handlers).
 *
 * Spins the executor for the specified timeout. Should be called once per loop().
 * Non-blocking; returns after timeout_ms or when no more messages.
 *
 * @param timeout_ms Timeout in milliseconds
 */
void interaction_micro_ros_spin(uint32_t timeout_ms);

/**
 * @brief Registers callback for emotion display commands.
 *
 * When a message arrives on /emotion/int, the provided callback is invoked
 * with the emotion index.
 *
 * @param callback Function pointer: void callback(int emotion_id)
 *
 * Topic: /emotion/int (std_msgs/msg/Int32)
 */
void interaction_micro_ros_set_emotion_cmd_callback(void (*callback)(int));

/** @} */

#endif
