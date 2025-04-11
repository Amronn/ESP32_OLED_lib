#include <Arduino.h>
#include <math.h>
#include <OLED_A.h>

const static float centerX = 63; // Środek ekranu OLED (128 px szerokości)
const static float centerY = 31; // Środek ekranu OLED (64 px wysokości)


#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

OLED_Display oled;
void tran(uint8_t arr[][2], size_t size, int dx, int dy) {
  for (size_t i = 0; i < size; i++) {
      arr[i][0] += dx;  // Modyfikacja wartości x (pierwsza kolumna)
      arr[i][1] += dy;  // Modyfikacja wartości x (pierwsza kolumna)
  }
}


void rot(float arr[][2], uint8_t size, float rotz) {
    const float radians = rotz * (M_PI / 180.0f); // Konwersja stopni na radiany
    
    // Obliczenie wartości trygonometrycznych tylko raz
    const float cosR = cosf(radians);
    const float sinR = sinf(radians);

    for (uint8_t i = 0; i < size; i++) {
        // Przesunięcie względem środka
        const float dx = arr[i][0] - centerX;
        const float dy = arr[i][1] - centerY;

        // Zastosowanie macierzy obrotu
        arr[i][0] = dx * cosR - dy * sinR + centerX;
        arr[i][1] = dx * sinR + dy * cosR + centerY;
    }
}

void rot3D(float arr[][3], uint8_t size, float rotx, float roty, float rotz) {
    const float radiansx = rotx * (M_PI / 180.0f);
    const float radiansy = roty * (M_PI / 180.0f);
    const float radiansz = rotz * (M_PI / 180.0f);

    // Obliczenie wartości trygonometrycznych tylko raz
    const float cosRx = cosf(radiansx), sinRx = sinf(radiansx);
    const float cosRy = cosf(radiansy), sinRy = sinf(radiansy);
    const float cosRz = cosf(radiansz), sinRz = sinf(radiansz);

    for (uint8_t i = 0; i < size; i++) {
        float x = arr[i][0];
        float y = arr[i][1];
        float z = arr[i][2];

        // Rotacja wokół X
        if (rotx != 0) {
            float newY = y * cosRx - z * sinRx;
            float newZ = y * sinRx + z * cosRx;
            y = newY;
            z = newZ;
        }

        // Rotacja wokół Y
        if (roty != 0) {
            float newX = x * cosRy + z * sinRy;
            float newZ = -x * sinRy + z * cosRy;
            x = newX;
            z = newZ;
        }

        // Rotacja wokół Z
        if (rotz != 0) {
            float newX = x * cosRz - y * sinRz;
            float newY = x * sinRz + y * cosRz;
            x = newX;
            y = newY;
        }

        arr[i][0] = x;
        arr[i][1] = y;
        arr[i][2] = z;
    }
}


void proj(float xyzs[][3], float xys[][2], uint8_t size, float d = 40) {
    

    for (uint8_t i = 0; i < size; i++) {
        float x = xyzs[i][0];
        float y = xyzs[i][1];
        float z = xyzs[i][2] + d; // Uniknięcie dzielenia przez 0

        xys[i][0] = (x * d) / z + centerX;
        xys[i][1] = (y * d) / z + centerY;
    }
}
// Krawędzie sześcianu
uint8_t edges[12][2] = {
  {0, 1}, {1, 2}, {2, 3}, {3, 0}, // Dolna podstawa
  {4, 5}, {5, 6}, {6, 7}, {7, 4}, // Górna podstawa
  {0, 4}, {1, 5}, {2, 6}, {3, 7}  // Połączenia góra-dół
};

// Pozycje rzutowane
float xys[8][2];

// Współrzędne 3D sześcianu
float xyzs[8][3] = {
  {0, 0, 0}, {20, 0, 0}, {20, 20, 0}, {0, 20, 0},  // Dolna podstawa
  {0, 0, 20}, {20, 0, 20}, {20, 20, 20}, {0, 20, 20} // Górna podstawa
};

// Flaga do synchronizacji rdzeni
volatile bool readyToDraw = false;

// Funkcja renderowania (działa na rdzeniu 1)
void TaskRendering(void *pvParameters) {
  while (true) {
    rot3D(xyzs, ARRAY_SIZE(xyzs), 0, 1, 1);
    proj(xyzs, xys, 8);
    // oled.drawFromFile("/obraz.txt");
    readyToDraw = true; // Powiadomienie głównego rdzenia
    vTaskDelay(16 / portTICK_PERIOD_MS);
  }
}
void setup() {
  Serial.begin(9600);
  oled.init();
  oled.clear();
  // oled.drawFromFile("/obraz.txt");
  // oled.drawFillRectangle(31,31-16, 64, 32, 0);
  oled.update();
  // oled.drawPixel(0,0, 1);
  // oled.drawPixel(127,0, 1);
  // Uruchomienie zadania na rdzeniu 1
  xTaskCreatePinnedToCore(
    TaskRendering, // Funkcja zadania
    "RenderingTask", // Nazwa
    1024, // Rozmiar stosu
    NULL, // Parametry
    1, // Priorytet
    NULL, // Uchwyt zadania
    1 // Rdzeń 1 (ESP32-S3)
  );
}

void loop() {
  if (readyToDraw) {
    oled.drawFromPoints(edges, xys, ARRAY_SIZE(edges), 1);
    oled.update();
    oled.drawFromPoints(edges, xys, ARRAY_SIZE(edges), 0);
    readyToDraw = false; // Reset flagi
  }
}
