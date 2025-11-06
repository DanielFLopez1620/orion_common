#ifndef SCREEN_HPP
#define SCREEN_HPP

#include <Arduino.h>
#include <TFT_22_ILI9225.h>
#include <SPI.h>

#define TFT_RST        26   // Reset pin
#define TFT_RS         25   // Data/Command pin
#define TFT_CS         15   // Chip Select pin
#define TFT_SDI        13   // MOSI pin
#define TFT_CLK        14   // SCK pin
#define TFT_LED        0    // Should be 0 if wired to +5V
#define TFT_BRIGHTNESS 200  // Brightness level
#define TIME_OUT 2500       // Time out


#define FELIZ_WIDTH  176
#define FELIZ_HEIGHT 220

class Screen{
    public:
        Screen();
        void initialize();
        void drawEmotion(int emotion);
        void drawBitmap(int x, int y, const unsigned char *bitmap, int w, int h, uint16_t color);
        void displayEmotion(int emotion);

    private:
        float scale = 2.0;
};


#endif