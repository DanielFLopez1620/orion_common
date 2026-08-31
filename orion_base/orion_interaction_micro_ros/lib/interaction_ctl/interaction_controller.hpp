/**
 * @file interaction_controller.hpp
 * @brief Interaction control logic (touch sensors + screen emotion display).
 *
 * Encapsulates all control logic for the ORION interaction module:
 * - Touch sensor reading (4 capacitive pads)
 * - Emotion display on the TFT screen (only redraws on change)
 * - Heartbeat timing (connectivity monitoring)
 *
 * NO micro-ROS dependencies — pure control logic, testable and reusable.
 */

#ifndef INTERACTION_CONTROLLER_HPP
#define INTERACTION_CONTROLLER_HPP

#include "touch_driver.hpp"
#include "screen.hpp"
#include "hardware.hpp"
#include "constants.hpp"

/**
 * @class InteractionController
 * @brief Manages touch sensors and screen emotion display.
 *
 * Responsibilities:
 * - Sample the four touch sensors once per update() call
 * - Draw the requested emotion on screen, only when it changes
 * - Track when a heartbeat is due to be published
 * - Expose sensor states for feedback publishing (e.g., to ROS)
 */
class InteractionController {
private:
    // Hardware drivers (composition)
    TouchSensorDriver touch_ur;
    TouchSensorDriver touch_ul;
    TouchSensorDriver touch_lr;
    TouchSensorDriver touch_ll;
    Screen screen;

    // State
    int current_emotion = -1;
    unsigned long last_heartbeat_time = 0;

public:
    /**
     * @brief Constructor: wires up touch sensors to their pins.
     */
    InteractionController();

    /**
     * @brief Initializes touch sensor pins and the screen; call once in setup().
     */
    void initialize();

    /**
     * @brief Safe startup: displays the default (happy) emotion.
     */
    void safeStartup();

    /**
     * @brief Main update: samples all touch sensors.
     * Call once per loop() iteration.
     */
    void update();

    /**
     * @brief Displays the given emotion on screen, only if it changed
     * since the last call.
     *
     * @param emotion_id Emotion index [0-7]: Angry, Disgust, Fear, Happy,
     *                    Neutral, Sad, Surprise, Wink.
     *                    Out-of-range values are ignored.
     */
    void setEmotion(int emotion_id);

    /** @return current state of the upper-right touch sensor. */
    bool getTouchUR() const;

    /** @return current state of the upper-left touch sensor. */
    bool getTouchUL() const;

    /** @return current state of the lower-right touch sensor. */
    bool getTouchLR() const;

    /** @return current state of the lower-left touch sensor. */
    bool getTouchLL() const;

    /**
     * @brief Checks whether a heartbeat is due, resetting the internal
     * timer if so.
     *
     * @return true if HEARTBEAT_INTERVAL_MS has elapsed since the last
     *         heartbeat (caller should publish one).
     */
    bool shouldPublishHeartbeat();
};

#endif
