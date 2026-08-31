/*
 * @file constants.hpp
 * @brief Timing constants for ORION interaction firmware.
 *
 * Centralizes touch sensor sampling rate and heartbeat publishing interval.
 * Adjust these values when tuning responsiveness or connectivity monitoring.
 */

#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

namespace interaction
{
    /* Timing constants for sensor sampling and heartbeat publishing.
       Used by InteractionController and the micro-ROS bridge timer. */
    struct TIMING
    {
        // Touch sensor sampling / ROS timer rate (ms) — 250 ms = 4 Hz
        static const unsigned int SENSOR_READ_RATE_MS {250};

        // Heartbeat publish interval (ms) — connectivity monitoring
        static const unsigned long HEARTBEAT_INTERVAL_MS {1000};
    };

    /* Emotion display constants. Must stay in sync with the bitmap arrays
       in lib/screen/emotions.hpp (epd_bitmap_allArray, emotion_color). */
    struct EMOTION
    {
        // Number of available emotion bitmaps; valid indices are [0, COUNT-1]:
        // Angry, Disgust, Fear, Happy, Neutral, Sad, Surprise, Wink
        static const int COUNT {8};

        // Emotion shown on startup (Happy).
        // NOTE: not named DEFAULT — the ESP32 Arduino core #defines that.
        static const int STARTUP {3};
    };
}

#endif
