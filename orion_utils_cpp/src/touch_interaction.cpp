/*
 * @file touch_interaction.cpp
 * @brief Touch-sensor interaction node for ORION robot.
 *
 * Maps the four capacitive touch sensors on ESP32 #2 to robot behaviors
 * combining arm gestures, emotion display, and spoken responses.
 *
 * Sensor → Behavior
 *   /interaction/touch_ur (upper-right) → Tickle / surprise reaction
 *   /interaction/touch_ul (upper-left)  → Greeting wave
 *   /interaction/touch_lr (lower-right) → Thoughtful pose
 *   /interaction/touch_ll (lower-left)  → Sad / tired reaction
 */

#include <atomic>
#include <chrono>
#include <functional>
#include <mutex>
#include <thread>
#include <unordered_map>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/bool.hpp"
#include "std_msgs/msg/float64_multi_array.hpp"
#include "std_msgs/msg/int32.hpp"
#include "std_msgs/msg/string.hpp"

using namespace std::chrono_literals;

// Arm joint position constants (radians) — joint limits are ±π/3 ≈ ±1.047
static constexpr double ARM_NEUTRAL =  0.0;
static constexpr double ARM_UP      =  1.0;
static constexpr double ARM_HALF    =  0.5;
static constexpr double ARM_DOWN    = -0.5;

// Emotion index restored after each action (neutral/default face)
static constexpr int EMOTION_DEFAULT = 3;

class TouchInteractionNode : public rclcpp::Node
{
public:
  TouchInteractionNode()
  : Node("touch_interaction_node"), busy_(false)
  {
    left_pub_    = create_publisher<std_msgs::msg::Float64MultiArray>(
      "/simple_left_arm_controller/commands",  10);
    right_pub_   = create_publisher<std_msgs::msg::Float64MultiArray>(
      "/simple_right_arm_controller/commands", 10);
    emotion_pub_ = create_publisher<std_msgs::msg::Int32>("/emotion/int",    10);
    tts_pub_     = create_publisher<std_msgs::msg::String>("/orion_response", 10);

    for (const auto & [sensor, topic] : std::unordered_map<std::string, std::string>{
        {"ur", "/interaction/touch_ur"},
        {"ul", "/interaction/touch_ul"},
        {"lr", "/interaction/touch_lr"},
        {"ll", "/interaction/touch_ll"}})
    {
      prev_[sensor] = false;
      subs_.push_back(
        create_subscription<std_msgs::msg::Bool>(
          topic, 10,
          [this, sensor](const std_msgs::msg::Bool::SharedPtr msg) {
            on_touch(msg, sensor);
          }));
    }

    RCLCPP_INFO(get_logger(), "Touch interaction node ready.");
  }

private:
  // ------------------------------------------------------------------
  // Low-level publishers
  // ------------------------------------------------------------------

  void arms(double left, double right)
  {
    std_msgs::msg::Float64MultiArray l, r;
    l.data = {left};
    r.data = {right};
    left_pub_->publish(l);
    right_pub_->publish(r);
  }

  void emotion(int idx)
  {
    std_msgs::msg::Int32 msg;
    msg.data = idx;
    emotion_pub_->publish(msg);
  }

  void say(const std::string & text)
  {
    std_msgs::msg::String msg;
    msg.data = text;
    tts_pub_->publish(msg);
  }

  // ------------------------------------------------------------------
  // Rising-edge detection and dispatch
  // ------------------------------------------------------------------

  void on_touch(const std_msgs::msg::Bool::SharedPtr msg, const std::string & sensor)
  {
    if (!msg->data) {
      prev_[sensor] = false;
      return;
    }
    if (prev_[sensor]) {
      return;  // still held — ignore until released
    }
    prev_[sensor] = true;

    if (busy_.exchange(true)) {
      return;  // another action is running
    }

    std::thread([this, sensor]() {
      static const std::unordered_map<
        std::string, std::function<void(TouchInteractionNode *)>> actions = {
        {"ur", &TouchInteractionNode::tickle},
        {"ul", &TouchInteractionNode::greet},
        {"lr", &TouchInteractionNode::think},
        {"ll", &TouchInteractionNode::sad},
      };
      actions.at(sensor)(this);
      busy_.store(false);
    }).detach();
  }

  // ------------------------------------------------------------------
  // Behaviors
  // ------------------------------------------------------------------

  /* Upper-right: tickle / surprise — both arms flail twice then reset. */
  void tickle()
  {
    emotion(4); say("¡Me haces cosquillas!");
    for (int i = 0; i < 2; ++i) {
      arms(ARM_UP, ARM_UP);     std::this_thread::sleep_for(350ms);
      arms(ARM_DOWN, ARM_DOWN); std::this_thread::sleep_for(250ms);
    }
    arms(ARM_NEUTRAL, ARM_NEUTRAL);
    std::this_thread::sleep_for(500ms);
    emotion(EMOTION_DEFAULT);
  }

  /* Upper-left: greeting — left arm waves twice, happy emotion. */
  void greet()
  {
    emotion(1); say("¡Hola! ¿Cómo estás?");
    for (int i = 0; i < 2; ++i) {
      arms(ARM_UP,   ARM_NEUTRAL); std::this_thread::sleep_for(450ms);
      arms(ARM_HALF, ARM_NEUTRAL); std::this_thread::sleep_for(350ms);
    }
    arms(ARM_NEUTRAL, ARM_NEUTRAL);
    std::this_thread::sleep_for(400ms);
    emotion(EMOTION_DEFAULT);
  }

  /* Lower-right: thoughtful — right arm rises slowly, neutral emotion. */
  void think()
  {
    emotion(2); say("Mmm... déjame pensar en eso.");
    arms(ARM_NEUTRAL, ARM_HALF); std::this_thread::sleep_for(800ms);
    arms(ARM_NEUTRAL, ARM_UP);   std::this_thread::sleep_for(1500ms);
    arms(ARM_NEUTRAL, ARM_NEUTRAL);
    std::this_thread::sleep_for(500ms);
    emotion(EMOTION_DEFAULT);
  }

  /* Lower-left: sad / tired — both arms droop, sad emotion. */
  void sad()
  {
    emotion(6); say("Eso no me gustó mucho...");
    arms(ARM_DOWN, ARM_DOWN);
    std::this_thread::sleep_for(2000ms);
    arms(ARM_NEUTRAL, ARM_NEUTRAL);
    std::this_thread::sleep_for(500ms);
    emotion(EMOTION_DEFAULT);
  }

  // ------------------------------------------------------------------
  // Members
  // ------------------------------------------------------------------

  rclcpp::Publisher<std_msgs::msg::Float64MultiArray>::SharedPtr left_pub_, right_pub_;
  rclcpp::Publisher<std_msgs::msg::Int32>::SharedPtr emotion_pub_;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr tts_pub_;
  std::vector<rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr> subs_;
  std::unordered_map<std::string, bool> prev_;
  std::atomic<bool> busy_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<TouchInteractionNode>());
  rclcpp::shutdown();
  return 0;
}
