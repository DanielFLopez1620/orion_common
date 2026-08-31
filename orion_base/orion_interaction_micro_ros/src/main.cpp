/**
 * @file main.cpp
 * @brief Interaction firmware for ORION robot ESP32 #2 (micro-ROS) —
 *        Refactored Entry Point
 *
 * This firmware orchestrates:
 * 1. Hardware initialization (touch sensors, TFT screen)
 * 2. Control logic (sensor sampling, emotion display, heartbeat timing)
 * 3. Micro-ROS communication (publishers, subscriber, callbacks)
 *
 * Architecture:
 * - lib/control/: Pure control logic (no ROS)
 * - lib/micro_ros_bridge/: All micro-ROS plumbing (encapsulated, no control logic)
 * - lib/touch + lib/screen: Low-level hardware abstractions
 * - main.cpp (this file): Orchestration — connects everything together
 *
 * The main loop is simple:
 *   1. Update control logic (sample touch sensors)
 *   2. Publish feedback (touch states, heartbeat)
 *   3. Process ROS messages (spin executor)
 *
 * Publishes:
 *   - /interaction/touch_ur (Bool): upper-right touch sensor state
 *   - /interaction/touch_ul (Bool): upper-left touch sensor state
 *   - /interaction/touch_lr (Bool): lower-right touch sensor state
 *   - /interaction/touch_ll (Bool): lower-left touch sensor state
 *   - /interaction/heartbeat (Bool): periodic connectivity heartbeat
 *
 * Subscribes to:
 *   - /emotion/int (Int32): emotion index [0-7] to display on screen
 */

#include <Arduino.h>

// Control logic (NO micro-ROS dependencies)
#include "interaction_controller.hpp"

// Micro-ROS bridge (encapsulates all ROS infrastructure)
#include "interaction_ros.hpp"

// ============================================================================
// Global Control Objects
// ============================================================================

InteractionController interaction_ctrl;

// ============================================================================
// Callbacks: Bridge between ROS and Control Logic
// ============================================================================

/**
 * Called by interaction_ros when /emotion/int is received.
 * Delegates to the interaction controller.
 */
void on_emotion_cmd(int emotion_id) {
    interaction_ctrl.setEmotion(emotion_id);
}

// ============================================================================
// Arduino Lifecycle Hooks
// ============================================================================

void setup() {
    // 1. Initialize hardware (touch sensors, screen)
    interaction_ctrl.initialize();

    // 2. Initialize micro-ROS (node, publishers, subscriber)
    interaction_micro_ros_init();

    // 3. Register ROS callbacks (connect ROS topics to control logic)
    interaction_micro_ros_set_emotion_cmd_callback(on_emotion_cmd);

    // 4. Safe startup (display default emotion)
    interaction_ctrl.safeStartup();
}

void loop() {
    // 1. Update control logic (sample touch sensors)
    interaction_ctrl.update();

    // 2. Publish touch states to ROS
    interaction_micro_ros_publish_touch(
        interaction_ctrl.getTouchUR(),
        interaction_ctrl.getTouchUL(),
        interaction_ctrl.getTouchLR(),
        interaction_ctrl.getTouchLL());

    // 3. Publish heartbeat if due
    if (interaction_ctrl.shouldPublishHeartbeat()) {
        interaction_micro_ros_publish_heartbeat();
    }

    // 4. Process ROS messages (execute subscriptions and timers)
    interaction_micro_ros_spin(100);

    // Delay required to avoid over-heating ESP32
    delay(100);
}
