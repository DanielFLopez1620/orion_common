/**
 * @file interaction_ros.cpp
 * @brief Micro-ROS bridge implementation for ORION interaction module.
 *
 * This file encapsulates ALL micro-ROS infrastructure (rclc support, node, executor,
 * publishers, subscriber, message buffers) in an anonymous namespace.
 */

#include "interaction_ros.hpp"

#include <Arduino.h>
#include <micro_ros_platformio.h>

#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <std_msgs/msg/bool.h>
#include <std_msgs/msg/int32.h>

#include "constants.hpp"

namespace {

    // ---- ROS Infrastructure (must not be accessed from outside this file)
    rcl_node_t node;
    rclc_support_t support;
    rcl_allocator_t allocator;
    rclc_executor_t executor;

    // ---- Publishers
    rcl_publisher_t pub_touch_ur;
    rcl_publisher_t pub_touch_ul;
    rcl_publisher_t pub_touch_lr;
    rcl_publisher_t pub_touch_ll;
    rcl_publisher_t pub_heartbeat;

    // ---- Subscriber
    rcl_subscription_t sub_emotion_cmd;

    // ---- Message buffers for publishers
    std_msgs__msg__Bool msg_touch_ur;
    std_msgs__msg__Bool msg_touch_ul;
    std_msgs__msg__Bool msg_touch_lr;
    std_msgs__msg__Bool msg_touch_ll;
    std_msgs__msg__Bool msg_heartbeat;

    // ---- Message buffer for subscriber
    std_msgs__msg__Int32 msg_emotion_cmd;

    // ---- Timer (keeps executor alive; sensor sampling is driven from loop())
    rcl_timer_t timer_sensor;

    // ---- User callbacks (function pointers registered via public API)
    void (*user_emotion_cmd_callback)(int) = nullptr;

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
     * ROS callback for emotion command subscription.
     * Unwraps the Int32 message and calls user callback if registered.
     */
    void ros_emotion_cmd_callback(const void *msgin) {
        const std_msgs__msg__Int32 *msg = (const std_msgs__msg__Int32 *)msgin;
        if (user_emotion_cmd_callback) {
            user_emotion_cmd_callback(msg->data);
        }
    }

    /**
     * Timer callback (keeps executor alive, no action needed).
     * Touch sampling/publishing is driven from main.cpp's loop(), not here.
     */
    void timer_sensor_callback(rcl_timer_t *timer, int64_t last_call_time) {
        RCLC_UNUSED(timer);
        RCLC_UNUSED(last_call_time);
    }

}  // namespace


void interaction_micro_ros_init() {
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
    } while (ret != RCL_RET_OK && (millis() - start < 120000));

    RCCHECK(rclc_node_init_default(&node, "orion_interaction_node", "", &support));

    // ---- Create publishers
    RCCHECK(rclc_publisher_init_default(
        &pub_touch_ur,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Bool),
        "interaction/touch_ur"));

    RCCHECK(rclc_publisher_init_default(
        &pub_touch_ul,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Bool),
        "interaction/touch_ul"));

    RCCHECK(rclc_publisher_init_default(
        &pub_touch_lr,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Bool),
        "interaction/touch_lr"));

    RCCHECK(rclc_publisher_init_default(
        &pub_touch_ll,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Bool),
        "interaction/touch_ll"));

    RCCHECK(rclc_publisher_init_default(
        &pub_heartbeat,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Bool),
        "interaction/heartbeat"));

    // ---- Create subscriber
    RCCHECK(rclc_subscription_init_default(
        &sub_emotion_cmd,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
        "/emotion/int"));

    // ---- Create timer (keeps executor alive at the sensor sampling rate)
    RCCHECK(rclc_timer_init_default2(
        &timer_sensor,
        &support,
        RCL_MS_TO_NS(interaction::TIMING::SENSOR_READ_RATE_MS),
        timer_sensor_callback,
        true));

    // ---- Initialize executor (1 timer + 1 subscription)
    RCCHECK(rclc_executor_init(&executor, &support.context, 2, &allocator));

    RCCHECK(rclc_executor_add_timer(&executor, &timer_sensor));

    RCCHECK(rclc_executor_add_subscription(
        &executor,
        &sub_emotion_cmd,
        &msg_emotion_cmd,
        &ros_emotion_cmd_callback,
        ON_NEW_DATA));

    Serial.println("[INTERACTION_ROS] Micro-ROS initialized successfully");
}

void interaction_micro_ros_publish_touch(bool ur, bool ul, bool lr, bool ll) {
    msg_touch_ur.data = ur;
    msg_touch_ul.data = ul;
    msg_touch_lr.data = lr;
    msg_touch_ll.data = ll;

    RCSOFTCHECK(rcl_publish(&pub_touch_ur, (const void *)&msg_touch_ur, NULL));
    RCSOFTCHECK(rcl_publish(&pub_touch_ul, (const void *)&msg_touch_ul, NULL));
    RCSOFTCHECK(rcl_publish(&pub_touch_lr, (const void *)&msg_touch_lr, NULL));
    RCSOFTCHECK(rcl_publish(&pub_touch_ll, (const void *)&msg_touch_ll, NULL));
}

void interaction_micro_ros_publish_heartbeat() {
    msg_heartbeat.data = true;
    RCSOFTCHECK(rcl_publish(&pub_heartbeat, (const void *)&msg_heartbeat, NULL));
}

void interaction_micro_ros_spin(uint32_t timeout_ms) {
    RCSOFTCHECK(rclc_executor_spin_some(&executor, RCL_MS_TO_NS(timeout_ms)));
}

void interaction_micro_ros_set_emotion_cmd_callback(void (*callback)(int)) {
    user_emotion_cmd_callback = callback;
}
