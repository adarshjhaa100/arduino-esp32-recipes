/* MODIFIED FOR: CYD (Cheap Yellow Display) - ESP32-2432S028R
    GOAL: Draw an interesting 100x100 pixel Cat
*/

#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// Initialize TFT and Sprite
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite img = TFT_eSprite(&tft); // Create a sprite (buffer)

// Touchscreen pins for CYD
#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

// --- COLOR DEFINITIONS (RGB565) ---
#define C_ORANGE    0xFD20
#define C_DARK_ORG  0xCA00
#define C_WHITE     0xFFFF
#define C_BLACK     0x0000
#define C_PINK      0xFE19
#define C_BG        0x2124 // Dark Grey Background

// Global variables for position
int catX = (SCREEN_WIDTH / 2) - 50;
int catY = (SCREEN_HEIGHT / 2) - 50;

// -------------------------------------------------------------------------
// FUNCTION: Draw a 100x100 Cat into the Sprite
// -------------------------------------------------------------------------
void drawCatSprite() {
  // 1. Clear the sprite with a background color
  img.fillSprite(C_BG);

  // --- EARS ---
  // Left Ear
  img.fillTriangle(10, 10, 35, 30, 10, 50, C_ORANGE);
  img.drawTriangle(10, 10, 35, 30, 10, 50, C_DARK_ORG); // Outline
  
  // Right Ear
  img.fillTriangle(90, 10, 65, 30, 90, 50, C_ORANGE);
  img.drawTriangle(90, 10, 65, 30, 90, 50, C_DARK_ORG); // Outline

  // --- HEAD ---
  // Main face circle
  img.fillCircle(50, 55, 40, C_ORANGE);
  img.drawCircle(50, 55, 40, C_DARK_ORG); // Outline

  // --- STRIPES (Forehead) ---
  img.fillTriangle(50, 20, 45, 35, 55, 35, C_DARK_ORG);
  img.fillTriangle(50, 25, 30, 40, 70, 40, C_ORANGE); // Masking to shape stripes

  // --- EYES ---
  // White parts
  img.fillEllipse(35, 50, 8, 10, C_WHITE);
  img.fillEllipse(65, 50, 8, 10, C_WHITE);
  // Pupils (Black)
  img.fillCircle(35, 50, 4, C_BLACK);
  img.fillCircle(65, 50, 4, C_BLACK);
  // Shine (Cute factor)
  img.fillCircle(37, 48, 2, C_WHITE);
  img.fillCircle(67, 48, 2, C_WHITE);

  // --- SNOUT & NOSE ---
  img.fillEllipse(50, 70, 12, 8, 0xFFF0); // Light cream snout
  img.fillTriangle(45, 68, 55, 68, 50, 76, C_PINK); // Pink Nose

  // --- WHISKERS ---
  img.drawLine(20, 65, 5, 60, C_BLACK);
  img.drawLine(20, 70, 5, 70, C_BLACK);
  img.drawLine(20, 75, 5, 80, C_BLACK);

  img.drawLine(80, 65, 95, 60, C_BLACK);
  img.drawLine(80, 70, 95, 70, C_BLACK);
  img.drawLine(80, 75, 95, 80, C_BLACK);

  // --- MOUTH ---
  img.drawSmoothArc(50, 76, 5, 4, 45, 135, C_BLACK, 0xFFF0, true);
}

void setup() {
  Serial.begin(115200);

  // Init Touchscreen
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  touchscreen.setRotation(1);

  // Init TFT
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(C_BG);

  // Init Sprite (Buffer) - 100x100 pixels
  // 16-bit color requires 2 bytes per pixel -> 100*100*2 = 20KB RAM (ESP32 handles this easily)
  img.setColorDepth(16);
  img.createSprite(100, 100);

  // Prepare the cat image in memory
  drawCatSprite();

  // Draw initial text
  tft.setTextColor(TFT_WHITE, C_BG);
  tft.drawCentreString("TOUCH TO MOVE CAT", SCREEN_WIDTH / 2, 10, 2);

  // Push the cat sprite to the center of screen
  img.pushSprite(catX, catY);
}

void loop() {
  if (touchscreen.tirqTouched() && touchscreen.touched()) {
    TS_Point p = touchscreen.getPoint();
    
    // Calibrate and Map coordinates
    int tx = map(p.x, 200, 3700, 1, SCREEN_WIDTH);
    int ty = map(p.y, 240, 3800, 1, SCREEN_HEIGHT);
    int tz = p.z;

    // Only move if pressure is sufficient to avoid noise
    if (tz > 100) {
      // Clear the OLD cat position by drawing a rectangle of background color
      // (This is faster than clearing the whole screen)
      tft.fillRect(catX, catY, 100, 100, C_BG);

      // Update position to center the cat on your finger
      catX = tx - 50; 
      catY = ty - 50;

      // Keep cat inside screen boundaries
      if (catX < 0) catX = 0;
      if (catY < 0) catY = 0;
      if (catX > SCREEN_WIDTH - 100) catX = SCREEN_WIDTH - 100;
      if (catY > SCREEN_HEIGHT - 100) catY = SCREEN_HEIGHT - 100;

      // Push the pre-drawn sprite to the NEW location
      img.pushSprite(catX, catY);
      
      // Small delay to prevent jitter
      delay(50);
    }
  }
}