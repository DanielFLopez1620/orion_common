/*
 * @file screen.hpp
 * @brief TFT ILI9225 screen driver for ORION interaction ESP32.
 *
 * Provides bitmap and geometric emotion rendering via the TFT_22_ILI9225
 * library. Pin assignments match the ORION interaction PCB (HSPI bus).
 */

#ifndef SCREEN_HPP
#define SCREEN_HPP

#include <Arduino.h>
#include <TFT_22_ILI9225.h>
#include <SPI.h>

// ======================= TFT ILI9225 Pin Definitions ========================
#define TFT_RST        26   // Reset pin
#define TFT_RS         25   // Data/Command pin
#define TFT_CS         15   // Chip Select (HSPI-SS0)
#define TFT_SDI        13   // MOSI (HSPI-MOSI)
#define TFT_CLK        14   // Clock (HSPI-SCK)
#define TFT_LED        0    // Backlight pin (0 = wired directly to 3.3V)
#define TFT_BRIGHTNESS 200  // Backlight PWM level [0-255]
#define TIME_OUT       2500 // Display operation timeout (ms)

// ======================= Bitmap Dimensions ==================================
#define FELIZ_WIDTH  176    // Emotion bitmap width (pixels)
#define FELIZ_HEIGHT 220    // Emotion bitmap height (pixels)

/*
 * TFT ILI9225 screen abstraction for ORION emotion display.
 * Supports both bitmap sprite rendering and geometric fallback drawing.
 */
class Screen
{
public:
    Screen();

    /*
     * Initializes the TFT display and clears the screen.
     * Must be called once during setup before any draw calls.
     */
    void initialize();

    /*
     * Renders the emotion bitmap sprite for the given emotion index.
     *
     * @param emotion Emotion index [0-7]: Angry, Disgust, Fear, Happy,
     *                Neutral, Sad, Surprise, Wink
     */
    void drawEmotion(int emotion);

    /*
     * Draws a 1-bit bitmap at the given position with the specified color.
     *
     * @param x     Top-left X coordinate (pixels)
     * @param y     Top-left Y coordinate (pixels)
     * @param bitmap Pointer to bitmap byte array (1 bit per pixel, row-major)
     * @param w     Bitmap width (pixels)
     * @param h     Bitmap height (pixels)
     * @param color 16-bit RGB565 foreground color
     */
    void drawBitmap(int x, int y, const unsigned char *bitmap,
                    int w, int h, uint16_t color);

    /*
     * Renders a geometric (non-bitmap) emotion face as a fallback.
     * Useful for debugging without the full bitmap array flashed.
     *
     * @param emotion Emotion index [0-7]
     */
    void displayEmotion(int emotion);

private:
    float scale = 2.0; // Geometric drawing scale factor (pixels per unit)
};


#endif
