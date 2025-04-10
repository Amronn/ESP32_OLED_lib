// OLED_Display.cpp
#include "OLED_A.h"
#include <Wire.h>
#include "SPIFFS.h"
// #define FORMAT_SPIFFS_IF_FAILED true
OLED_Display::OLED_Display() {
    memset(buffer, 0, sizeof(buffer));
}

void OLED_Display::sendCommand(uint8_t cmd) {
    Wire.beginTransmission(OLED_I2C_ADDR);
    Wire.write(0x00);
    Wire.write(cmd);
    Wire.endTransmission();
}

void OLED_Display::init() {
    Wire.begin(4, 5);
    Wire.setClock(1000000);
    sendCommand(0xAE);
    sendCommand(0xD5);
    sendCommand(0x80);
    sendCommand(0xA8);
    sendCommand(0x3F);
    sendCommand(0xD3);
    sendCommand(0x00);
    sendCommand(0x40);
    sendCommand(0x8D);
    sendCommand(0x14);
    sendCommand(0xA1);
    sendCommand(0xC8);
    sendCommand(0xDA);
    sendCommand(0x12);
    sendCommand(0x81);
    sendCommand(0x7F);
    sendCommand(0xD9);
    sendCommand(0xF1);
    sendCommand(0xDB);
    sendCommand(0x40);
    sendCommand(0xA4);
    sendCommand(0xA6);
    sendCommand(0xAF);
    sendCommand(0x20);
    sendCommand(0x00);
    clear();
}

void OLED_Display::clear() {
    memset(buffer, 0, sizeof(buffer));
    update();
}

void OLED_Display::update() {
    const uint8_t width = 128;
    for (uint8_t page = 0; page < 8; page++) {
        sendCommand(0xB0 + page);
        sendCommand(0x02);
        sendCommand(0x10);

        uint8_t *ptr = buffer + (page * width);
        for (uint8_t col = 0; col < width; col += 64) {
            Wire.beginTransmission(OLED_I2C_ADDR);
            Wire.write(0x40);
            for (uint8_t i = 0; i < 64 && (col + i) < width; i++) {
                Wire.write(ptr[col + i]);
            }
            Wire.endTransmission();
        }
    }
}

void OLED_Display::drawPixel(uint8_t x, uint8_t y, bool color) {
    if (x >= 128 || y >= 64) return;
    uint8_t *ptr = &buffer[x + (y >> 3) * 128];
    uint8_t mask = 1 << (y & 7);
    color ? (*ptr |= mask) : (*ptr &= ~mask);
}

void OLED_Display::drawFromFile(const char* filename) {
    if (!SPIFFS.begin(true)) {
        Serial.println("Blad montowania SPIFFS");
        return;
    }

    File file = SPIFFS.open(filename, "r");
    if (!file) {
        Serial.println("Nie udalo się otworzyc pliku");
        return;
    }

    // clear();  // Czyści ekran przed rysowaniem

    uint8_t x = 0, y = 0;
    while (file.available() && y < 64) {
        char c = file.read();
        if (c == '0' || c == '1') {
            drawPixel(x, y, c == '1');  // Rysowanie piksela w zależności od wartości
            x++;
            // if (x >= 128) { // Przejście do nowego wiersza
            //     x = 0;
            //     y++;
            // }
        } else if (c == '\n') {
            x = 0; // Reset x na początku nowej linii
            y++;
        }
    }

    file.close();
    // update();  // Aktualizacja wyświetlacza
}


void OLED_Display::drawLine(int8_t x1, int8_t y1, int8_t x2, int8_t y2, bool color) {
    bool steep = abs(y2 - y1) > abs(x2 - x1); // Sprawdzenie, czy linia jest bardziej pionowa

    if (steep) { // Zamiana współrzędnych, jeśli linia jest stroma
        int8_t temp = x1; x1 = y1; y1 = temp;
        temp = x2; x2 = y2; y2 = temp;
    }

    if (x1 > x2) { // Zamiana punktów, jeśli x1 jest większe niż x2
        int8_t temp = x1; x1 = x2; x2 = temp;
        temp = y1; y1 = y2; y2 = temp;
    }

    uint8_t dx = x2 - x1;
    uint8_t dy = abs(y2 - y1);
    int8_t err = dx / 2;
    int8_t ystep = (y1 < y2) ? 1 : -1;
    int8_t y = y1;

    for (int8_t x = x1; x <= x2; x++) {
        if (steep) {
            drawPixel(y, x, color); // Jeśli linia była stroma, zamieniamy x i y
        } else {
            drawPixel(x, y, color);
        }
        err -= dy;
        if (err < 0) {
            y += ystep;
            err += dx;
        }
    }
}

void OLED_Display::drawRectangle(uint8_t x, uint8_t y, uint8_t width, uint8_t height, bool color) {
    drawLine(x, y, x + width, y, color);
    drawLine(x + width, y, x + width, y + height, color);
    drawLine(x, y + height, x + width, y + height, color);
    drawLine(x, y, x, y + height, color);
}

void OLED_Display::drawFillRectangle(uint8_t x, uint8_t y, uint8_t width, uint8_t height, bool color) {
    for(uint8_t i = 0; i < width; i++)
    {
        drawLine(x + i, y, x + i, y + height, color);
    }
}

void OLED_Display::drawFromPoints(uint8_t vertices[][2], float points[][2], uint8_t size, bool color) {
    for (uint8_t i = 0; i < size; i++) {
        drawLine(roundf(points[vertices[i][0]][0]), roundf(points[vertices[i][0]][1]),
                 roundf(points[vertices[i][1]][0]), roundf(points[vertices[i][1]][1]), color);
    }
}
