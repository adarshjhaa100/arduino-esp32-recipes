/* MINI DOOM (Raycaster Engine) for ESP32 CYD
   Based on Lodev's Raycasting Algorithm
   
   HARDWARE: ESP32-2432S028R (Cheap Yellow Display)
*/

#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// --- HARDWARE SETUP ---
TFT_eSPI tft = TFT_eSPI();

#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

// --- GAME SETTINGS ---
#define SCREEN_W 320
#define SCREEN_H 240
#define RENDER_W 160  // Raycasting resolution (width / 2) for performance
#define MAP_SIZE 24

// Colors
#define C_BLACK  0x0000 // Black
#define C_SKY    0x3186 // Dark Blue
#define C_FLOOR  0x3183 // Dark Grey
#define C_WALL_1 0xB800 // Red
#define C_WALL_2 0x8000 // Dark Red (for shading)
#define C_GUN    0x52AA // Gun Metal

// Map: 1 = Wall, 0 = Empty
// A simple arena with some pillars
const int worldMap[MAP_SIZE][MAP_SIZE] = {
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,1,1,1,1,1,0,0,0,0,0,0,0,1,0,1,0,1,0,0,0,1},
  {1,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,1,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1},
  {1,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,1,1,0,1,1,0,0,0,0,0,0,0,1,0,1,0,1,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1},
  {1,0,0,0,1,0,0,0,0,1,1,1,1,1,0,0,0,0,1,0,0,0,0,1},
  {1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

// Player State
double posX = 22.0, posY = 12.0;  // Start position
double dirX = -1.0, dirY = 0.0;   // Direction vector
double planeX = 0.0, planeY = 0.66; // Camera plane (Field of View)

// Movement Speed
double moveSpeed = 0.15;
double rotSpeed = 0.10;

void setup() {
  Serial.begin(115200);

  // Init Touch
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  touchscreen.setRotation(1);

  // Init TFT
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(C_BLACK);
}

void drawGun() {
  // Simple blocky gun at bottom center
  int gunW = 40;
  int gunH = 50;
  int gunX = (SCREEN_W / 2) - (gunW / 2);
  int gunY = SCREEN_H - gunH;

  // Draw Gun Body
  tft.fillRect(gunX, gunY, gunW, gunH, C_GUN);
  tft.fillRect(gunX + 10, gunY - 10, 20, 10, C_GUN); // Barrel tip
  
  // Muzzle Flash logic (optional, add flash state later)
}

void handleInput() {
  if (touchscreen.touched()) {
    TS_Point p = touchscreen.getPoint();
    // Map Touch Coordinates
    int tx = map(p.x, 200, 3700, 1, SCREEN_W);
    int ty = map(p.y, 240, 3800, 1, SCREEN_H);
    
    // --- CONTROLS ---
    // Left/Right strips (20% width) = TURN
    if (tx < 60) {
      // Rotate Right (inverted logic due to camera plane usually)
      double oldDirX = dirX;
      dirX = dirX * cos(rotSpeed) - dirY * sin(rotSpeed);
      dirY = oldDirX * sin(rotSpeed) + dirY * cos(rotSpeed);
      double oldPlaneX = planeX;
      planeX = planeX * cos(rotSpeed) - planeY * sin(rotSpeed);
      planeY = oldPlaneX * sin(rotSpeed) + planeY * cos(rotSpeed);
    } 
    else if (tx > 260) {
      // Rotate Left
      double oldDirX = dirX;
      dirX = dirX * cos(-rotSpeed) - dirY * sin(-rotSpeed);
      dirY = oldDirX * sin(-rotSpeed) + dirY * cos(-rotSpeed);
      double oldPlaneX = planeX;
      planeX = planeX * cos(-rotSpeed) - planeY * sin(-rotSpeed);
      planeY = oldPlaneX * sin(-rotSpeed) + planeY * cos(-rotSpeed);
    }
    // Center Top = MOVE FORWARD
    else if (ty < SCREEN_H / 2) {
      if(worldMap[int(posX + dirX * moveSpeed)][int(posY)] == 0) posX += dirX * moveSpeed;
      if(worldMap[int(posX)][int(posY + dirY * moveSpeed)] == 0) posY += dirY * moveSpeed;
    }
    // Center Bottom = MOVE BACKWARD
    else {
      if(worldMap[int(posX - dirX * moveSpeed)][int(posY)] == 0) posX -= dirX * moveSpeed;
      if(worldMap[int(posX)][int(posY - dirY * moveSpeed)] == 0) posY -= dirY * moveSpeed;
    }
  }
}

void renderScene() {
  // Raycasting Loop
  for (int x = 0; x < RENDER_W; x++) {
    // Calculate ray position and direction
    double cameraX = 2 * x / double(RENDER_W) - 1; // x-coordinate in camera space
    double rayDirX = dirX + planeX * cameraX;
    double rayDirY = dirY + planeY * cameraX;

    // Which box of the map we're in
    int mapX = int(posX);
    int mapY = int(posY);

    // Length of ray from current position to next x or y-side
    double sideDistX;
    double sideDistY;

    // Length of ray from one x or y-side to next x or y-side
    double deltaDistX = (rayDirX == 0) ? 1e30 : abs(1 / rayDirX);
    double deltaDistY = (rayDirY == 0) ? 1e30 : abs(1 / rayDirY);
    double perpWallDist;

    // What direction to step in x or y-direction (either +1 or -1)
    int stepX;
    int stepY;

    int hit = 0; // Was there a wall hit?
    int side;    // Was a NS or a EW wall hit?

    // Calculate step and initial sideDist
    if (rayDirX < 0) {
      stepX = -1;
      sideDistX = (posX - mapX) * deltaDistX;
    } else {
      stepX = 1;
      sideDistX = (mapX + 1.0 - posX) * deltaDistX;
    }
    if (rayDirY < 0) {
      stepY = -1;
      sideDistY = (posY - mapY) * deltaDistY;
    } else {
      stepY = 1;
      sideDistY = (mapY + 1.0 - posY) * deltaDistY;
    }

    // Perform DDA (Digital Differential Analyzer)
    while (hit == 0) {
      // Jump to next map square, OR in x-direction, OR in y-direction
      if (sideDistX < sideDistY) {
        sideDistX += deltaDistX;
        mapX += stepX;
        side = 0;
      } else {
        sideDistY += deltaDistY;
        mapY += stepY;
        side = 1;
      }
      // Check if ray has hit a wall
      if (worldMap[mapX][mapY] > 0) hit = 1;
    }

    // Calculate distance projected on camera direction
    if (side == 0) perpWallDist = (sideDistX - deltaDistX);
    else           perpWallDist = (sideDistY - deltaDistY);

    // Calculate height of line to draw on screen
    int lineHeight = (int)(SCREEN_H / perpWallDist);

    // Calculate lowest and highest pixel to fill in current stripe
    int drawStart = -lineHeight / 2 + SCREEN_H / 2;
    if (drawStart < 0) drawStart = 0;
    int drawEnd = lineHeight / 2 + SCREEN_H / 2;
    if (drawEnd >= SCREEN_H) drawEnd = SCREEN_H - 1;

    // Choose wall color
    uint16_t color = C_WALL_1;
    if (side == 1) color = C_WALL_2; // Shaded side

    // Draw the vertical line (scaled horizontally by 2 to fill 320px)
    // 1. Draw Ceiling (Sky)
    tft.fillRect(x * 2, 0, 2, drawStart, C_SKY);
    // 2. Draw Wall
    tft.fillRect(x * 2, drawStart, 2, drawEnd - drawStart, color);
    // 3. Draw Floor
    tft.fillRect(x * 2, drawEnd, 2, SCREEN_H - drawEnd, C_FLOOR);
  }
}

void loop() {
  handleInput();
  renderScene();
  drawGun();
  
  // Basic FPS Counter (optional, comment out for speed)
  // tft.setCursor(0, 0);
  // tft.setTextColor(TFT_WHITE, C_BLACK);
  // tft.print("FPS"); 
}