/**
 * @file main.cpp
 * @brief Actuator control firmware for ORION robot ESP32 (micro-ROS)
 *
 * Implements micro-ROS communication for:
 * - DC motor control (left/right) via PID feedback loops
 * - Encoder reading for odometry
 * - Servo motor positioning (left/right arms)
 *
 * Publishes:
 *   - /diff_ctl_left_enc (Int64): left encoder count
 *   - /diff_ctl_right_enc (Int64): right encoder count
 *   - /fwd_servo_left_feedback (Float32): left servo position (radians)
 *   - /fwd_servo_right_feedback (Float32): right servo position (radians)
 *
 * Subscribes to:
 *   - /diff_ctl_motor_cmd (Int64MultiArray): [left_speed, right_speed]
 *   - /fwd_servo_left_cmd (Float32): servo command (radians)
 *   - /fwd_servo_right_cmd (Float32): servo command (radians)
 */

#include <Arduino.h>
#include <micro_ros_platformio.h>

#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <std_msgs/msg/int64_multi_array.h>
#include <std_msgs/msg/multi_array_dimension.h>
#include <std_msgs/msg/int64.h>
#include <std_msgs/msg/float32.h>

#include "constants.hpp"
#include "encoder.hpp"
#include "hardware.hpp"
#include "motor.hpp"
#include "pid.hpp"
#include "servo.hpp"

#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){error_loop();}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){}}

void IRAM_ATTR read_left_enc();
void IRAM_ATTR read_right_enc();
void adjust_motors_speed();
void set_motor_speed(int left_speed, int right_speed);
void error_loop();
void safe_startup_pins();
void safe_startup();
void timer_diff_callback(rcl_timer_t * timer, int64_t last_call_tm);
void timer_fwd_callback(rcl_timer_t * timer, int64_t last_call_tm);
void cmd_motor_callback(const void *msgin);
void cmd_servo_left_callback(const void *msgin);
void cmd_servo_right_callback(const void *msgin);

unsigned const int max_servo_pos = 150;
unsigned const int min_servo_pos = 30;

rcl_publisher_t enc_left_pub;
rcl_publisher_t enc_right_pub;
rcl_publisher_t servo_left_pub;
rcl_publisher_t servo_right_pub;
rcl_subscription_t motor_speed_sub;
rcl_subscription_t servo_left_sub;
rcl_subscription_t servo_right_sub;

rclc_executor_t executor;
rclc_support_t support;
rcl_allocator_t allocator;
rcl_node_t node;
rcl_timer_t timer_diff;
rcl_timer_t timer_fwd;

std_msgs__msg__Int64 enc_left_value;
std_msgs__msg__Int64 enc_right_value;
std_msgs__msg__Float32 servo_left_cmd;
std_msgs__msg__Float32 servo_right_cmd;
std_msgs__msg__Float32 servo_left_feedback;
std_msgs__msg__Float32 servo_right_feedback;
std_msgs__msg__Int64MultiArray cmd_msg_speed;

bool received_motor_cmd = false;
unsigned long last_cmd_time = 0;
const unsigned long cmd_timeout_ms = 1000;

diff::MotorDriver motor_left(
    diff::HARDWARE::ML_EN, diff::HARDWARE::ML_FORW, diff::HARDWARE::ML_BACW);
diff::MotorDriver motor_right(
    diff::HARDWARE::MR_EN, diff::HARDWARE::MR_FORW, diff::HARDWARE::MR_BACW);

diff::EncoderDriver enc_left(diff::HARDWARE::ML_ENCA, diff::HARDWARE::ML_ENCB);
diff::EncoderDriver enc_right(diff::HARDWARE::MR_ENCA, diff::HARDWARE::MR_ENCB);

diff::ControlPID pid_left(
    diff::ROBOT_CONST::PID_KP, diff::ROBOT_CONST::PID_KD, diff::ROBOT_CONST::PID_KI,
    diff::ROBOT_CONST::PID_KO, diff::ROBOT_CONST::PWM_MAX, diff::ROBOT_CONST::PWM_MIN);
diff::ControlPID pid_right(
    diff::ROBOT_CONST::PID_KP, diff::ROBOT_CONST::PID_KD, diff::ROBOT_CONST::PID_KI,
    diff::ROBOT_CONST::PID_KO, diff::ROBOT_CONST::PWM_MAX, diff::ROBOT_CONST::PWM_MIN);

fwd::ServoMotor servo_left(max_servo_pos, min_servo_pos, fwd::HARDWARE::SERVO_LEFT);
fwd::ServoMotor servo_right(max_servo_pos, min_servo_pos, fwd::HARDWARE::SERVO_RIGHT);

// ---- SETUP ----
void setup()
{
    safe_startup_pins();
    motor_left.safeInit();
    motor_right.safeInit();

    Serial.begin(115200);
    set_microros_serial_transports(Serial);

    delay(2000);

    allocator = rcl_get_default_allocator();

    unsigned long start = millis();
    rcl_ret_t ret;
    do {
        ret = rclc_support_init(&support, 0, NULL, &allocator);
        motor_left.stop();
        motor_right.stop();
        if (ret != RCL_RET_OK) {
            delay(500);
        }
    } while (ret != RCL_RET_OK && (millis() - start < 180000));

    RCCHECK(rclc_node_init_default(&node, "micro_ros_platformio_orion_ctl_node", "", &support));

    RCCHECK(rclc_publisher_init_default(&enc_left_pub, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int64), "diff_ctl_left_enc"));
    RCCHECK(rclc_publisher_init_default(&enc_right_pub, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int64), "diff_ctl_right_enc"));
    RCCHECK(rclc_publisher_init_default(&servo_left_pub, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float32), "fwd_servo_left_feedback"));
    RCCHECK(rclc_publisher_init_default(&servo_right_pub, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float32), "fwd_servo_right_feedback"));

    RCCHECK(rclc_subscription_init_default(&motor_speed_sub, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int64MultiArray), "diff_ctl_motor_cmd"));
    RCCHECK(rclc_subscription_init_default(&servo_left_sub, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float32), "fwd_servo_left_cmd"));
    RCCHECK(rclc_subscription_init_default(&servo_right_sub, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float32), "fwd_servo_right_cmd"));

    const unsigned int timer_timeout = 1000 / diff::ROBOT_CONST::PID_RATE;
    const unsigned int servo_timeout = 100;

    RCCHECK(rclc_timer_init_default2(&timer_diff, &support, RCL_MS_TO_NS(timer_timeout), timer_diff_callback, true));
    RCCHECK(rclc_timer_init_default2(&timer_fwd, &support, RCL_MS_TO_NS(servo_timeout), timer_fwd_callback, true));

    cmd_msg_speed.data.capacity = 2;
    cmd_msg_speed.data.size = 0;
    cmd_msg_speed.data.data = (int64_t*) malloc(cmd_msg_speed.data.capacity * sizeof(int64_t));

    RCCHECK(rclc_executor_init(&executor, &support.context, 5, &allocator));
    RCCHECK(rclc_executor_add_timer(&executor, &timer_diff));
    RCCHECK(rclc_executor_add_timer(&executor, &timer_fwd));
    RCCHECK(rclc_executor_add_subscription(&executor, &motor_speed_sub, &cmd_msg_speed, &cmd_motor_callback, ON_NEW_DATA));
    RCCHECK(rclc_executor_add_subscription(&executor, &servo_left_sub, &servo_left_cmd, &cmd_servo_left_callback, ON_NEW_DATA));
    RCCHECK(rclc_executor_add_subscription(&executor, &servo_right_sub, &servo_right_cmd, &cmd_servo_right_callback, ON_NEW_DATA));

    motor_left.begin();
    motor_right.begin();
    pid_left.disable();
    pid_right.disable();
    pid_left.setSetpoint(0);
    pid_right.setSetpoint(0);

    enc_left.begin();
    enc_right.begin();
    servo_left.begin();
    servo_right.begin();


    // Single-channel quadrature decode: only ENCA triggers the ISR; ENCB is
    // sampled inside the ISR via digitalRead(). This halves the effective
    // encoder resolution compared to full quadrature (both edges, both channels).
    attachInterrupt(diff::HARDWARE::ML_ENCA, &read_left_enc, CHANGE);
    attachInterrupt(diff::HARDWARE::MR_ENCA, &read_right_enc, CHANGE);

    safe_startup();
}

// ---- LOOP ----
void loop()
{
    if (received_motor_cmd && (millis() - last_cmd_time > cmd_timeout_ms)) {
        motor_left.setSpeed(0);
        motor_right.setSpeed(0);
        pid_left.disable();
        pid_right.disable();
        received_motor_cmd = false;
    }

    RCSOFTCHECK(rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100)));
}

void safe_startup() {
    motor_left.setSpeed(0);
    motor_right.setSpeed(0);
    pid_left.disable();
    pid_right.disable();

    delay(500);
    enc_left.reset();
    enc_right.reset();

    pid_left.reset(enc_left.read());
    pid_right.reset(enc_right.read());

    servo_left.setPositionRad(M_PI_2);
    servo_right.setPositionRad(M_PI_2);

    delay(500);
}

// ---- FUNCTION DEFINITIONS -----

/*
 * Initializes all motor/servo pins as outputs and sets them to LOW.
 * Ensures safe startup without accidental motor movement.
 */
void safe_startup_pins()
{
    pinMode(diff::HARDWARE::ML_FORW, OUTPUT);
    pinMode(diff::HARDWARE::ML_BACW, OUTPUT);
    pinMode(diff::HARDWARE::MR_FORW, OUTPUT);
    pinMode(diff::HARDWARE::MR_BACW, OUTPUT);
    pinMode(diff::HARDWARE::ML_EN, OUTPUT);
    pinMode(diff::HARDWARE::MR_EN, OUTPUT);
    pinMode(fwd::HARDWARE::SERVO_LEFT, OUTPUT);
    pinMode(fwd::HARDWARE::SERVO_RIGHT, OUTPUT);

    digitalWrite(diff::HARDWARE::ML_EN, LOW);
    digitalWrite(diff::HARDWARE::MR_EN, LOW);
    digitalWrite(diff::HARDWARE::ML_FORW, LOW);
    digitalWrite(diff::HARDWARE::ML_BACW, LOW);
    digitalWrite(diff::HARDWARE::MR_FORW, LOW);
    digitalWrite(diff::HARDWARE::MR_BACW, LOW);
    digitalWrite(fwd::HARDWARE::SERVO_LEFT, LOW);
    digitalWrite(fwd::HARDWARE::SERVO_RIGHT, LOW);
}


/**
 * ISR for left encoder pulse (attached to ML_ENCA, single-channel quadrature) 
 */
void IRAM_ATTR read_left_enc() { enc_left.readEnc(); }

/*
 * ISR for right encoder pulse (attached to MR_ENCA, single-channel quadrature) 
*/
void IRAM_ATTR read_right_enc() { enc_right.readEnc(); }

/*
 * Computes PID output and updates motor speeds based on encoder feedback.
 * Called periodically by timer_diff_callback.
 */
void adjust_motors_speed()
{
    if (!received_motor_cmd) return;

    int motor_left_sp = 0;
    int motor_right_sp = 0;

    pid_left.compute(enc_left.read(), motor_left_sp);
    pid_right.compute(enc_right.read(), motor_right_sp);

    if(pid_left.enabled()) motor_left.setSpeed(motor_left_sp);
    if(pid_right.enabled()) motor_right.setSpeed(motor_right_sp);
}

/*
 * Sets motor speed setpoints and enables/disables PID controllers.
 * @param left_speed target encoder count change per cycle (0 to stop)
 * @param right_speed target encoder count change per cycle (0 to stop)
 */
void set_motor_speed(int left_speed, int right_speed)
{
    if(left_speed == 0) {
        motor_left.setSpeed(0);
        pid_left.disable();
    } else {
        pid_left.enable();
    }

    if(right_speed == 0) {
        motor_right.setSpeed(0);
        pid_right.disable();
    } else {
        pid_right.enable();
    }

    pid_left.setSetpoint((float)left_speed / (float)diff::ROBOT_CONST::PID_RATE);
    pid_right.setSetpoint((float)right_speed / (float)diff::ROBOT_CONST::PID_RATE);
}

/*
 * Halts execution with LED blink pattern and serial error message.
 * Called when micro-ROS initialization fails.
 */
void error_loop()
{
    pinMode(LED_BUILTIN, OUTPUT);
    while(1)
    {
        Serial.println("[ERROR] micro-ROS init failed — halted.");
        digitalWrite(LED_BUILTIN, HIGH);
        delay(200);
        digitalWrite(LED_BUILTIN, LOW);
        delay(200);
    }
}

/*
 * Timer callback for differential drive control (called at PID rate).
 * Computes motor speeds via PID and publishes encoder feedback.
 */
void timer_diff_callback(rcl_timer_t * timer, int64_t last_call_tm)
{
    RCLC_UNUSED(last_call_tm);
    if(timer != NULL)
    {
        adjust_motors_speed();
        enc_left_value.data = enc_left.read();
        enc_right_value.data = enc_right.read();

        RCSOFTCHECK(rcl_publish(&enc_left_pub, (const void*)&enc_left_value, NULL));
        RCSOFTCHECK(rcl_publish(&enc_right_pub, (const void*)&enc_right_value, NULL));
    }
}

/*
 * Callback for motor speed commands from /diff_ctl_motor_cmd.
 * Expects Int64MultiArray with [left_speed, right_speed].
 */
void cmd_motor_callback(const void *msgin)
{
    const std_msgs__msg__Int64MultiArray * msg = (const std_msgs__msg__Int64MultiArray *) msgin;

    if (msg->data.size != 2) {
        return;
    }

    received_motor_cmd = true;
    last_cmd_time = millis();
    set_motor_speed(msg->data.data[0], msg->data.data[1]);
}

/*
 * Timer callback for servo feedback publishing.
 * Publishes current servo positions at fixed rate.
 */
void timer_fwd_callback(rcl_timer_t * timer, int64_t last_call_tm)
{
    RCLC_UNUSED(last_call_tm);
    if(timer != NULL)
    {
        float left_pos = servo_left.getPositionRad() - M_PI_2;
        float right_pos = servo_right.getPositionRad() - M_PI_2;
        servo_left_feedback.data = left_pos;
        servo_right_feedback.data = right_pos;
        RCSOFTCHECK(rcl_publish(&servo_left_pub, (const void*)&servo_left_feedback, NULL));
        RCSOFTCHECK(rcl_publish(&servo_right_pub, (const void*)&servo_right_feedback, NULL));
    }
}

/*
 * Callback for left servo command from /fwd_servo_left_cmd.
 * Expects Float32 position in radians (centered at 0, range ~[-π/2, π/2]).
 */
void cmd_servo_left_callback(const void *msgin)
{
    const std_msgs__msg__Float32 *msg = (const std_msgs__msg__Float32 *)msgin;
    servo_left.setPositionRad((float)msg->data + M_PI_2);
}

/*
 * Callback for right servo command from /fwd_servo_right_cmd.
 * Expects Float32 position in radians (centered at 0, range ~[-π/2, π/2]).
 */
void cmd_servo_right_callback(const void *msgin)
{
    const std_msgs__msg__Float32 *msg = (const std_msgs__msg__Float32 *)msgin;
    servo_right.setPositionRad((float)msg->data + M_PI_2);
}
