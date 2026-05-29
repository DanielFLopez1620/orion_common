#include "screen.hpp"
#include "emotions.hpp"

SPIClass hspi(HSPI);
TFT_22_ILI9225 tft = TFT_22_ILI9225(TFT_RST, TFT_RS, TFT_CS, TFT_SDI, TFT_CLK,
    TFT_LED, TFT_BRIGHTNESS);

Screen::Screen() {}

void Screen::initialize()
{
	hspi.begin(TFT_CLK, -1, TFT_SDI, TFT_CS); // SCK, MISO, MOSI, SS
	tft.begin(hspi);

	tft.clear();
	tft.setBackgroundColor(COLOR_WHITE);
	tft.setFont(Terminal12x16);
}

void Screen::drawEmotion(int emotion)
{
	tft.clear();
	tft.drawBitmap(0, 0, epd_bitmap_allArray[emotion], 176, 220, emotion_color[emotion]);
}


void Screen::drawBitmap(int x, int y, const unsigned char *bitmap, int w, int h, uint16_t color)
{
	tft.clear();
	for (int j = 0; j < h; j++)
	{
		for (int i = 0; i < w; i++)
		{
			int byteIndex = i + (j / 8) * w;
			bool pixelOn = bitRead(pgm_read_byte(bitmap + byteIndex), j % 8);
			if (pixelOn)
			{
				tft.drawPixel(x + i, y + j, color);
			}
		}
	}
}

void Screen::displayEmotion(int emotion)
{
	switch((int)emotion)
    {
        // ------------------- Angry
        case 0:
            tft.clear();
            tft.fillRectangle(
                X_INI, Y1_INI,
                X_END, Y1_INI + Y_SIZE,
                COLOR_ORANGE
            );
            tft.fillRectangle(
                X_INI, Y2_INI, 
                X_END, Y2_INI + Y_SIZE,
                COLOR_ORANGE
            );
            tft.fillTriangle(
                X_END, Y1_INI, 
                X_END + 12, 
                Y1_INI, X_END,
                Y1_INI + Y_SIZE, 
                COLOR_ORANGE
            );
            tft.fillTriangle(
                X_END,
                Y2_INI, 
                X_END,
                Y2_INI + Y_SIZE, 
                X_END + 12,
                Y2_INI + Y_SIZE,
                COLOR_ORANGE);
            break;

        // ------------------- Inexpressive
        case 1:
            tft.clear();
            tft.fillRectangle(
                X_INI + X_OFF,
                Y1_INI, X_END - X_OFF,
                Y1_INI + Y_SIZE, 
                COLOR_YELLOW
            );
            tft.fillRectangle(
                X_INI + X_OFF,
                Y2_INI, X_END - X_OFF,
                Y2_INI + Y_SIZE,
                COLOR_YELLOW
            );
            break;

        // ------------------- Fear
        case 2:
            tft.clear();
            tft.fillCircle(
                (X_END + X_INI)/2,
                Y1_INI + Y_SIZE/2,
                Y_SIZE/3,
                COLOR_BLUE);
            tft.fillCircle(
                (X_END + X_INI)/2,
                Y2_INI + Y_SIZE/2,
                Y_SIZE/3,
                COLOR_BLUE);
            break;

        // ------------------- Happy
        case 3:
            tft.clear();
            tft.fillTriangle(
                X_INI,
                Y1_INI,
                X_INI,
                Y1_INI + Y_SIZE
                , X_END,
                (Y1_INI * 2 + Y_SIZE)/2,
                COLOR_YELLOW
            );
            tft.fillTriangle(
                X_INI,
                Y2_INI,
                X_INI,
                Y2_INI + Y_SIZE
                , X_END,
                (Y2_INI * 2 + Y_SIZE)/2,
                COLOR_YELLOW
            );
            tft.fillTriangle(
                X_INI,
                Y1_INI + 10,
                X_INI,
                Y1_INI + Y_SIZE - 10,
                X_END - 10,
                (Y1_INI * 2 + Y_SIZE)/2,
                COLOR_BLACK
            );
            tft.fillTriangle(
                X_INI,
                Y2_INI + 10,
                X_INI,
                Y2_INI + Y_SIZE - 10,
                X_END - 10,
                (Y2_INI * 2 + Y_SIZE)/2,
                COLOR_BLACK
            );
            break;

        // ------------------- Neutral
        case 4:
            tft.clear();
            tft.fillRectangle(X_INI,
                Y1_INI,
                X_END,
                Y1_INI + Y_SIZE,
                COLOR_YELLOW
            );
            tft.fillRectangle(X_INI,
                Y2_INI,
                X_END,
                Y2_INI + Y_SIZE,
                COLOR_YELLOW
            );
            break;

        // ------------------- Surprise
        case 5:
            tft.clear();
            tft.fillCircle(
                (X_END + X_INI)/2,
                Y1_INI + Y_SIZE/2,
                Y_SIZE/2,
                COLOR_YELLOW
            );
            tft.fillCircle(
                (X_END + X_INI)/2,
                Y2_INI + Y_SIZE/2,
                Y_SIZE/2,
                COLOR_YELLOW
            );
            break;

        // ------------------- Sad
        case 6:
            tft.clear();
            tft.fillRectangle(
                X_INI,
                Y1_INI,
                X_END,
                Y1_INI + Y_SIZE,
                COLOR_LIGHTBLUE);
            tft.fillRectangle(
                X_INI,
                Y2_INI,
                X_END,
                Y2_INI + Y_SIZE,
                COLOR_LIGHTBLUE);
            tft.fillTriangle(X_END,
                Y1_INI,
                X_END,
                Y1_INI + Y_SIZE,
                X_END + 10,
                Y1_INI + Y_SIZE,
                COLOR_LIGHTBLUE);
            tft.fillTriangle(X_END,
                Y2_INI,
                X_END + 10,
                Y2_INI, X_END,
                Y2_INI + Y_SIZE,
                COLOR_LIGHTBLUE);
            break;

        // ------------------- Default
        default:
            tft.clear();
            tft.fillRectangle(
                X_INI,
                Y1_INI,
                X_END,
                Y1_INI + Y_SIZE,
                COLOR_YELLOW
            );
            tft.fillRectangle(
                X_INI,
                Y2_INI,
                X_END,
                Y2_INI + Y_SIZE,
                COLOR_YELLOW
            );
            tft.fillCircle(
                X_INI,
                Y1_INI + Y_SIZE/2,
                Y_SIZE/2,
                COLOR_YELLOW
            );
            tft.fillCircle(
                X_END,
                Y1_INI + Y_SIZE/2,
                Y_SIZE/2,
                COLOR_YELLOW
            );
            tft.fillCircle(
                X_INI,
                Y2_INI + Y_SIZE/2,
                Y_SIZE/2,
                COLOR_YELLOW
            );
            tft.fillCircle(
                X_END,
                Y2_INI + Y_SIZE/2,
                Y_SIZE/2,
                COLOR_YELLOW
            );
            break;
        };
}