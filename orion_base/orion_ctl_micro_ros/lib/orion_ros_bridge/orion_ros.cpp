/**
 * @file orion_ros.cpp
 * @brief Micro-ROS bridge implementation for ORION.
 *
 * This file encapsulates ALL micro-ROS infrastructure (rclc support, node, executor,
 * publishers, subscribers, message buffers) in an anonymous namespace.
 *
 */

#include "orion_ros.hpp"

#include <Arduino.h>
#include <micro_ros_platformio.h>

#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <std_msgs/msg/int64_multi_array.h>
#include <std_msgs/msg/multi_array_dimension.h>
#include <std_msgs/msg/int64.h>
#include <std_msgs/msg/float32.h>

namespace {

    // ---- ROS Infrastructure (must not be accessed from outside this file)
    rcl_node_t node;
    rclc_support_t support;
    rcl_allocator_t allocator;
    rclc_executor_t executor;

    // ---- Publishers
    rcl_publisher_t pub_enc_left;
    rcl_publisher_t pub_enc_right;
    rcl_publisher_t pub_servo_left_feedback;
    rcl_publisher_t pub_servo_right_feedback;

    // ---- Subscribers
    rcl_subscription_t sub_motor_cmd;
    rcl_subscription_t sub_servo_left_cmd;
    rcl_subscription_t sub_servo_right_cmd;

    // ---- Message buffers for publishers
    std_msgs__msg__Int64 msg_enc_left;
    std_msgs__msg__Int64 msg_enc_right;
    std_msgs__msg__Float32 msg_servo_left_feedback;
    std_msgs__msg__Float32 msg_servo_right_feedback;

    // ---- Message buffers for subscribers
    std_msgs__msg__Int64MultiArray msg_motor_cmd;
    std_msgs__msg__Float32 msg_servo_left_cmd;
    std_msgs__msg__Float32 msg_servo_right_cmd;

    // ---- Timers (used to keep executor alive)
    rcl_timer_t timer_diff;
    rcl_timer_t timer_servo;

    // ---- User callbacks (function pointers registered via public API)
    void (*user_motor_cmd_callback)(int, int) = nullptr;
    void (*user_servo_left_callback)(float) = nullptr;
    void (*user_servo_right_callback)(float) = nullptr;

    // ---- Error handling
    #define RCCHECK(fn) \
        { \
            rcl_ret_t temp_rc = fn; \
            if ((temp_rc != RCL_RET_OK)) { \
                error_loop(); \
            } \
        }
    #define RCSOFTCHECK(fn) \
        { \
            rcl_ret_t temp_rc = fn; \
            if ((temp_rc != RCL_RET_OK)) { \
            } \
        }

    void error_loop() {
        pinMode(LED_BUILTIN, OUTPUT);
        while (1) {
            Serial.println("[ERROR] Micro-ROS init failed — halted.");
            digitalWrite(LED_BUILTIN, HIGH);
            delay(200);
            digitalWrite(LED_BUILTIN, LOW);
            delay(200);
        }
    }

    // ---- ROS Callback Wrappers (bridge between ROS and user callbacks)

    /**
     * ROS callback for motor speed command subscription.
     * Unwraps the Int64MultiArray message and calls user callback if registered.
     */
    void ros_motor_cmd_callback(const void *msgin) {
        const std_msgs__msg__Int64MultiArray *msg = (const std_msgs__msg__Int64MultiArray *)msgin;

        if (msg->data.size != 2) {
            return;  // Invalid message format
        }

        if (user_motor_cmd_callback) {
            user_motor_cmd_callback((int)msg->data.data[0], (int)msg->data.data[1]);
        }
    }

    /**
     * ROS callback for left servo command subscription.
     */
    void ros_servo_left_cmd_callback(const void *msgin) {
        const std_msgs__msg__Float32 *msg = (const std_msgs__msg__Float32 *)msgin;
        if (user_servo_left_callback) {
            user_servo_left_callback(msg->data);
        }
    }

    /**
     * ROS callback for right servo command subscription.
     */
    void ros_servo_right_cmd_callback(const void *msgin) {
        const std_msgs__msg__Float32 *msg = (const std_msgs__msg__Float32 *)msgin;
        if (user_servo_right_callback) {
            user_servo_right_callback(msg->data);
        }
    }

    /**
     * Timer callbacks (keep executor alive, no action needed).
     */
    void timer_diff_callback(rcl_timer_t *timer, int64_t last_call_time) {
        RCLC_UNUSED(timer);
        RCLC_UNUSED(last_call_time);
    }

    void timer_servo_callback(rcl_timer_t *timer, int64_t last_call_time) {
        RCLC_UNUSED(timer);
        RCLC_UNUSED(last_call_time);
    }

}  // namespace


void orion_micro_ros_init() {
    Serial.begin(115200);
    set_microros_serial_transports(Serial);

    delay(2000);

    allocator = rcl_get_default_allocator();

    // Retry logic: keep trying to initialize support until timeout
    unsigned long start = millis();
    rcl_ret_t ret;
    do {
        ret = rclc_support_init(&support, 0, NULL, &allocator);
        if (ret != RCL_RET_OK) {
            delay(500);
        }
    } while (ret != RCL_RET_OK && (millis() - start < 180000));

    RCCHECK(rclc_node_init_default(&node, "orion_ctl_node", "", &support));

    // ---- Create publishers
    RCCHECK(rclc_publisher_init_default(
        &pub_enc_left,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int64),
        "diff_ctl_left_enc"));

    RCCHECK(rclc_publisher_init_default(
        &pub_enc_right,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int64),
        "diff_ctl_right_enc"));

    RCCHECK(rclc_publisher_init_default(
        &pub_servo_left_feedback,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float32),
        "fwd_servo_left_feedback"));

    RCCHECK(rclc_publisher_init_default(
        &pub_servo_right_feedback,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float32),
        "fwd_servo_right_feedback"));

    // ---- Create subscribers
    RCCHECK(rclc_subscription_init_default(
        &sub_motor_cmd,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int64MultiArray),
        "diff_ctl_motor_cmd"));

    RCCHECK(rclc_subscription_init_default(
        &sub_servo_left_cmd,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float32),
        "fwd_servo_left_cmd"));

    RCCHECK(rclc_subscription_init_default(
        &sub_servo_right_cmd,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float32),
        "fwd_servo_right_cmd"));

    // ---- Create timers (keep executor alive)
    const unsigned int timer_timeout_diff = 1000 / 50;  // 50 Hz (PID rate)
    const unsigned int timer_timeout_servo = 100;       // 10 Hz

    RCCHECK(rclc_timer_init_default2(
        &timer_diff,
        &support,
        RCL_MS_TO_NS(timer_timeout_diff),
        timer_diff_callback,
        true));

    RCCHECK(rclc_timer_init_default2(
        &timer_servo,
        &support,
        RCL_MS_TO_NS(timer_timeout_servo),
        timer_servo_callback,
        true));

    // ---- Setup message buffers
    msg_motor_cmd.data.capacity = 2;
    msg_motor_cmd.data.size = 0;
    msg_motor_cmd.data.data = (int64_t *)malloc(msg_motor_cmd.data.capacity * sizeof(int64_t));

    // ---- Initialize executor
    RCCHECK(rclc_executor_init(&executor, &support.context, 5, &allocator));

    // ---- Add timers and subscriptions to executor
    RCCHECK(rclc_executor_add_timer(&executor, &timer_diff));
    RCCHECK(rclc_executor_add_timer(&executor, &timer_servo));

    RCCHECK(rclc_executor_add_subscription(
        &executor,
        &sub_motor_cmd,
        &msg_motor_cmd,
        &ros_motor_cmd_callback,
        ON_NEW_DATA));

    RCCHECK(rclc_executor_add_subscription(
        &executor,
        &sub_servo_left_cmd,
        &msg_servo_left_cmd,
        &ros_servo_left_cmd_callback,
        ON_NEW_DATA));

    RCCHECK(rclc_executor_add_subscription(
        &executor,
        &sub_servo_right_cmd,
        &msg_servo_right_cmd,
        &ros_servo_right_cmd_callback,
        ON_NEW_DATA));

    Serial.println("[ORION_ROS] Micro-ROS initialized successfully");
}

void orion_micro_ros_publish_encoders(int64_t left_count, int64_t right_count) {
    msg_enc_left.data = left_count;
    msg_enc_right.data = right_count;

    RCSOFTCHECK(rcl_publish(&pub_enc_left, (const void *)&msg_enc_left, NULL));
    RCSOFTCHECK(rcl_publish(&pub_enc_right, (const void *)&msg_enc_right, NULL));
}

void orion_micro_ros_publish_servo_feedback(float left_rad, float right_rad) {
    msg_servo_left_feedback.data = left_rad;
    msg_servo_right_feedback.data = right_rad;

    RCSOFTCHECK(rcl_publish(&pub_servo_left_feedback, (const void *)&msg_servo_left_feedback, NULL));
    RCSOFTCHECK(rcl_publish(&pub_servo_right_feedback, (const void *)&msg_servo_right_feedback, NULL));
}

void orion_micro_ros_spin(uint32_t timeout_ms) {
    RCSOFTCHECK(rclc_executor_spin_some(&executor, RCL_MS_TO_NS(timeout_ms)));
}

void orion_micro_ros_set_motor_cmd_callback(void (*callback)(int, int)) {
    user_motor_cmd_callback = callback;
}

void orion_micro_ros_set_servo_left_cmd_callback(void (*callback)(float)) {
    user_servo_left_callback = callback;
}

void orion_micro_ros_set_servo_right_cmd_callback(void (*callback)(float)) {
    user_servo_right_callback = callback;
}
