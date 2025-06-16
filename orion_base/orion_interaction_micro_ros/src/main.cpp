// ////////////////////// DEPENDENCIES AND LIBRARIES //////////////////////////
// ---------------------- Required Arduino Libraries --------------------------
#include <Arduino.h>
#include <SPI.h>
#include <TFT_22_ILI9225.h>

// ---------------------- Platformio Libraries --------------------------------
#include <micro_ros_platformio.h>

// ---------------------- ROS Client Library for C Libs -----------------------
#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

// ------------------------ Required messages ---------------------------------
#include <std_msgs/msg/bool.h>
#include <std_msgs/msg/int32.h>

// ----------------------- Cutom requierements -------------------------------
#include "screen.hpp"

// //////////////////////// GLOBAL DEFINITIONS ////////////////////////////////

// --------------------------- Definitions ------------------------------------

// Define touch sensors pints
#define TS_UR_PIN 4         // Upper right
#define TS_UL_PIN 34        // Upper left
#define TS_LR_PIN 2         // Lower right
#define TS_LL_PIN 35        // Lower left

// ------------------------- ROS 2 related definitions ------------------------
// Define publishers
rcl_publisher_t ts_ur_publisher;
rcl_publisher_t ts_ul_publisher;
rcl_publisher_t ts_lr_publisher;
rcl_publisher_t ts_ll_publisher;

// Define susbscribers
rcl_subscription_t emotion_subscriber;

// Define messages
std_msgs__msg__Bool ts_ur_msg;
std_msgs__msg__Bool ts_ul_msg;
std_msgs__msg__Bool ts_lr_msg;
std_msgs__msg__Bool ts_ll_msg;
std_msgs__msg__Int32 emotion_msg;

// Define executor
rclc_executor_t executor;

// Definte supporter
rclc_support_t support;

// Define memory allocator
rcl_allocator_t allocator;

// Define node
rcl_node_t node;

// Define timer
rcl_timer_t timer;

// Time tools
unsigned long last_ping_time = 0;
const unsigned long ping_interval_ms = 1000; 

// Helpers
int previous_emotion = 0;

// Screen
Screen screen;

// Define a ROS 2 Checker
#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){error_loop();}}

// Define a soft ROS 2 Checker
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){}}

// ///////////////////////////// FUNTION PROTOTYPES ///////////////////////////

void error_loop();
void timer_callback(rcl_timer_t * timer, int64_t last_call_time);
void emotion_callback(const void *msgin);

// ///////////////////// SINGLE SET UP FUNCTION ///////////////////////////////
void setup() 
{
	// Configure serial transport
	Serial.begin(115200);
	set_microros_serial_transports(Serial);
	delay(2000);

	// Initialize allocator
	allocator = rcl_get_default_allocator();

    // Retry agent connection
    unsigned long start = millis();
    rcl_ret_t ret;
    do {
        ret = rclc_support_init(&support, 0, NULL, &allocator);
        if (ret != RCL_RET_OK) 
        {
            delay(500);
        }
    } while (ret != RCL_RET_OK && (millis() - start < 120000));

	// Create node
	RCCHECK(rclc_node_init_default(&node, "micro_ros_platformio_touch_node", "", 
		&support));

	// Create publishers
	RCCHECK(rclc_publisher_init_default(
		&ts_ur_publisher,
		&node,
		ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Bool),
		"interaction/touch_ur"));
    
    RCCHECK(rclc_publisher_init_default(
		&ts_ul_publisher,
		&node,
		ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Bool),
		"interaction/touch_ul"));
    
    RCCHECK(rclc_publisher_init_default(
		&ts_lr_publisher,
		&node,
		ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Bool),
		"interaction/touch_lr"));
    
    RCCHECK(rclc_publisher_init_default(
		&ts_ll_publisher,
		&node,
		ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Bool),
		"interaction/touch_ll"));

    // Create subscriber
    RCCHECK(rclc_subscription_init_default(
		&emotion_subscriber,
		&node,
		ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
		"/emotion/int"));

	// Create timer,
	const unsigned int timer_timeout = 250;
	RCCHECK(rclc_timer_init_default2(
		&timer,
		&support,
		RCL_MS_TO_NS(timer_timeout),
		timer_callback,
        true));

	// Initialize executor
	RCCHECK(rclc_executor_init(&executor, &support.context, 2, &allocator));

    // Add timer and subscriber to the executor
	RCCHECK(rclc_executor_add_timer(&executor, &timer));
    RCCHECK(rclc_executor_add_subscription(
        &executor,
        &emotion_subscriber,
        &emotion_msg,
        &emotion_callback,
        ON_NEW_DATA));


    // Configure pins
    pinMode(TS_LL_PIN, INPUT);
    pinMode(TS_LR_PIN,INPUT);
    pinMode(TS_UL_PIN,INPUT);
    pinMode(TS_UR_PIN,INPUT);

    // Initialize screen
    screen.initialize();

} // void setup()

// /////////////////////////// LOOP IMPLEMENTATION ///////////////////////////
void loop() 
{
    // Delay required to avoid over-heating ESP32
	delay(100);

	// Spin
	RCSOFTCHECK(rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100)));

} // void loop()

// ///////////////////////// FUNCTION DEFINITIONS ////////////////////////////

/**
 * Loop to handle errors
 */
void error_loop() 
{
	while(1) 
	{
		delay(100);
	}
} // void error_loop()

/**
 * Function that will be linked to the timer in order to publish
 * the message data.
 * 
 * @param timer Pointer to timer object
 * @param last_call_time Last time the timer was called
 */
void timer_callback(rcl_timer_t * timer, int64_t last_call_time) 
{
	RCLC_UNUSED(last_call_time);
	if (timer != NULL) 
	{
		// Check state button when timer is called
        ts_ur_msg.data = digitalRead(TS_UR_PIN);
        ts_ul_msg.data = digitalRead(TS_UL_PIN);
        ts_lr_msg.data = digitalRead(TS_LR_PIN);
        ts_ll_msg.data = digitalRead(TS_LL_PIN);

        // Publish states
		RCSOFTCHECK(rcl_publish(&ts_ur_publisher, &ts_ur_msg, NULL));
        RCSOFTCHECK(rcl_publish(&ts_ul_publisher, &ts_ul_msg, NULL));
        RCSOFTCHECK(rcl_publish(&ts_lr_publisher, &ts_lr_msg, NULL));
        RCSOFTCHECK(rcl_publish(&ts_ll_publisher, &ts_ll_msg, NULL));
	}
} // void timer_callback()

/**
 * Callback that manages std_msgs/msgs/Int32 in order to desplay an emotion
 * on screen.
 * 
 * @param msgin Pointer to the message received
 */
void emotion_callback(const void *msgin)
{
    const std_msgs__msg__Int32 * emotion = (const std_msgs__msg__Int32 *) msgin;
    int value = emotion->data;
    if(previous_emotion != value)
    {
        // Bitmap emotion
        screen.drawEmotion(value);

        // Geometry emotion
        // screen.displayEmotion(value);

        // Update emotion
        previous_emotion = value;
    }
    
} // emotion_callback()