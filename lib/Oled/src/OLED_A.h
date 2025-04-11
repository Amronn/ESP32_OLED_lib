// OLED_Display.h
#ifndef OLED_A_H
#define OLED_A_H

#include <Arduino.h>

#define OLED_I2C_ADDR 0x3C

class OLED_Display {
private:
    uint8_t buffer[128 * 8] = {0};
    void sendCommand(uint8_t cmd);

public:
    OLED_Display();
    void init(uint8_t sda, uint8_t scl); // Updated to accept SDA and SCL pins
    void clear();
    void update();
    void drawPixel(uint8_t x, uint8_t y, bool color);
    void drawLine(int8_t x1, int8_t y1, int8_t x2, int8_t y2, bool color);
    void drawRectangle(uint8_t x, uint8_t y, uint8_t width, uint8_t height, bool color);
    void drawFillRectangle(uint8_t x, uint8_t y, uint8_t width, uint8_t height, bool color);
    void drawFromFile(const char* filename);
    void drawFromPoints(uint8_t vertices[][2], float points[][2], uint8_t size, bool color);
};

#endif