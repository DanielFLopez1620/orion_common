#include "interaction_controller.hpp"

#include <Arduino.h>

InteractionController::InteractionController()
    : touch_ur(interaction::HARDWARE::TS_UR_PIN),
      touch_ul(interaction::HARDWARE::TS_UL_PIN),
      touch_lr(interaction::HARDWARE::TS_LR_PIN),
      touch_ll(interaction::HARDWARE::TS_LL_PIN)
{
}

void InteractionController::initialize()
{
    touch_ur.initialize();
    touch_ul.initialize();
    touch_lr.initialize();
    touch_ll.initialize();

    screen.initialize();
}

void InteractionController::safeStartup()
{
    setEmotion(interaction::EMOTION::STARTUP);
}

void InteractionController::update()
{
    touch_ur.read();
    touch_ul.read();
    touch_lr.read();
    touch_ll.read();
}

void InteractionController::setEmotion(int emotion_id)
{
    // Guard the bitmap array bounds: drawEmotion() indexes straight into
    // epd_bitmap_allArray, so an out-of-range id would read invalid memory.
    if (emotion_id < 0 || emotion_id >= interaction::EMOTION::COUNT)
    {
        return;
    }

    if (current_emotion != emotion_id)
    {
        screen.drawEmotion(emotion_id);
        current_emotion = emotion_id;
    }
}

bool InteractionController::getTouchUR() const
{
    return touch_ur.getState();
}

bool InteractionController::getTouchUL() const
{
    return touch_ul.getState();
}

bool InteractionController::getTouchLR() const
{
    return touch_lr.getState();
}

bool InteractionController::getTouchLL() const
{
    return touch_ll.getState();
}

bool InteractionController::shouldPublishHeartbeat()
{
    if (millis() - last_heartbeat_time >= interaction::TIMING::HEARTBEAT_INTERVAL_MS)
    {
        last_heartbeat_time = millis();
        return true;
    }
    return false;
}
