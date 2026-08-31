/**
 * @file main.cpp
 * @brief ORION Differential Drive Control Firmware — Refactored Entry Point
 *
 * This firmware orchestrates:
 * 1. Hardware initialization (motors, encoders, servos)
 * 2. Control logic (PID, speed regulation, command timeouts)
 * 3. Micro-ROS communication (publishers, subscribers, callbacks)
 *
 * Architecture:
 * - lib/control/: Pure control logic (no ROS)
 * - lib/micro_ros_bridge/: All micro-ROS plumbing (encapsulated, no control logic)
 * - lib/hardware + drivers: Low-level hardware abstractions
 * - main.cpp (this file): Orchestration — connects everything together
 *
 * The main loop is simple:
 *   1. Update control logic (motors, PID)
 *   2. Publish feedback (encoders, servos)
 *   3. Process ROS messages (spin executor)
 */

#include <Arduino.h>

// Control logic (NO micro-ROS dependencies)
#include "differential_drive_controller.hpp"
#include "servo_controller.hpp"

// Micro-ROS bridge (encapsulates all ROS infrastructure)
#include "orion_ros.hpp"

// Hardware pin definitions and tuning constants
#include "hardware.hpp"
#include "constants.hpp"

// ============================================================================
// Global Control Objects
// ============================================================================

DifferentialDriveController drive_ctrl(
    diff::HARDWARE::ML_EN, diff::HARDWARE::ML_FORW, diff::HARDWARE::ML_BACW,
    diff::HARDWARE::MR_EN, diff::HARDWARE::MR_FORW, diff::HARDWARE::MR_BACW,
    diff::HARDWARE::ML_ENCA, diff::HARDWARE::ML_ENCB,
    diff::HARDWARE::MR_ENCA, diff::HARDWARE::MR_ENCB);

ServoController servo_ctrl(
    fwd::HARDWARE::SERVO_LEFT,
    fwd::HARDWARE::SERVO_RIGHT);

// ============================================================================
// Callbacks: Bridge between ROS and Control Logic
// ============================================================================

/**
 * Called by orion_ros when /diff_ctl_motor_cmd is received.
 * Delegates to the drive controller.
 */
void on_motor_cmd(int left_speed, int right_speed) {
    drive_ctrl.setTargetSpeed(left_speed, right_speed);
}

/**
 * Called by orion_ros when /fwd_servo_left_cmd is received.
 * Delegates to the servo controller.
 */
void on_servo_left_cmd(float radians) {
    servo_ctrl.setLeftPosition(radians);
}

/**
 * Called by orion_ros when /fwd_servo_right_cmd is received.
 * Delegates to the servo controller.
 */
void on_servo_right_cmd(float radians) {
    servo_ctrl.setRightPosition(radians);
}

// ============================================================================
// ISR Forward Declarations (defined at the bottom of this file)
// ============================================================================

void IRAM_ATTR isr_left_encoder();
void IRAM_ATTR isr_right_encoder();

// ============================================================================
// Arduino Lifecycle Hooks
// ============================================================================

void setup() {
    // 1. Initialize hardware (motors, encoders, servos)
    drive_ctrl.initialize();
    servo_ctrl.initialize();

    // 2. Initialize micro-ROS (node, publishers, subscribers)
    orion_micro_ros_init();

    // 3. Register ROS callbacks (connect ROS topics to control logic)
    orion_micro_ros_set_motor_cmd_callback(on_motor_cmd);
    orion_micro_ros_set_servo_left_cmd_callback(on_servo_left_cmd);
    orion_micro_ros_set_servo_right_cmd_callback(on_servo_right_cmd);

    // 4. Safe startup (stop motors, reset encoders, center servos)
    drive_ctrl.safeStartup();

    // 5. Attach encoder ISRs (quadrature decoding on rising/falling edges)
    attachInterrupt(diff::HARDWARE::ML_ENCA, isr_left_encoder, CHANGE);
    attachInterrupt(diff::HARDWARE::MR_ENCA, isr_right_encoder, CHANGE);
}

void loop() {
    // 1. Update control logic (adjust motor speeds via PID)
    drive_ctrl.update();

    // 2. Check for command timeouts (safety: stop motors if no command received)
    drive_ctrl.checkTimeout();

    // 3. Publish feedback to ROS (encoder counts and servo positions)
    orion_micro_ros_publish_encoders(
        drive_ctrl.getLeftEncoderCount(),
        drive_ctrl.getRightEncoderCount());

    orion_micro_ros_publish_servo_feedback(
        servo_ctrl.getLeftPosition(),
        servo_ctrl.getRightPosition());

    // 4. Process ROS messages (execute subscriptions and timers)
    orion_micro_ros_spin(100);
}

// ============================================================================
// ISR Handlers (Encoder Edge Detection)
// ============================================================================

/**
 * Interrupt Service Routine for left encoder edge.
 * Calls the drive controller to update encoder state.
 */
void IRAM_ATTR isr_left_encoder() {
    drive_ctrl.onLeftEncoderEdge();
}

/**
 * Interrupt Service Routine for right encoder edge.
 * Calls the drive controller to update encoder state.
 */
void IRAM_ATTR isr_right_encoder() {
    drive_ctrl.onRightEncoderEdge();
}
