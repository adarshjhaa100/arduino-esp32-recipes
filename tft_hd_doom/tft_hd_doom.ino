/* ESP32 RayCaster - Zero Flicker Version
   Hardware: ESP32-2432S028R (CYD)
*/

#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// --- Hardware Definitions ---
#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

// --- Screen Settings ---
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

// --- PERFORMANCE TUNING ---
#define RES_DIVIDER 2 

// --- Minimap Settings ---
#define MM_SCALE 3      
#define MM_OFFSET_X 5   
#define MM_OFFSET_Y 5   
// Calculate total map size: (24 * 3) + 5 = 77 pixels. We round up to 80 for safety.
#define MM_SAFE_WIDTH 82 
#define MM_SAFE_HEIGHT 82

// --- Soft Color Palette (5-6-5 format hex) ---
#define C_SKY     0xCE79 
#define C_FLOOR   0x2965 
#define C_WALL1   0xFCC0 
#define C_WALL2   0x867F 
#define C_WALL3   0x97F1 
#define C_WALL4   0xCE59 
#define C_UI      0xF800 
#define C_MM_WALL 0x8410 
#define C_MM_BG   0x0000 

// --- Game Map (1 = Wall, 0 = Empty) ---
#define MAP_WIDTH 24
#define MAP_HEIGHT 24

const uint8_t worldMap[MAP_WIDTH][MAP_HEIGHT] = {
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,1,1,1,1,1,0,0,0,0,3,0,3,0,3,0,0,0,0,0,1},
  {1,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,1,0,0,0,1,0,0,0,0,3,0,0,0,3,0,0,0,0,0,1},
  {1,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,1,1,0,1,1,0,0,0,0,3,0,3,0,3,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,2,0,0,0,0,1},
  {1,4,0,0,0,0,5,0,4,0,0,0,0,0,0,0,0,0,2,0,0,0,0,1},
  {1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,2,0,0,0,0,1},
  {1,4,0,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

// --- Player State ---
float posX = 22.0f, posY = 12.0f;  
float dirX = -1.0f, dirY = 0.0f;   
float planeX = 0.0f, planeY = 0.66f; 

// Tuned for better response
float moveSpeed = 0.12f; 
float rotSpeed = 0.10f;

// --- Objects ---
TFT_eSPI tft = TFT_eSPI(); 
SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

void setup() {
  Serial.begin(115200);

  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  touchscreen.setRotation(1);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  
  // Draw the static parts of the UI once
  drawMinimapGrid();
}

void loop() {
  handleInput();
  renderView();
}

void drawUI() {
  // We only redraw the arrows if they were damaged, but for now, 
  // redrawing them is fast enough if we don't erase the background.
  tft.fillTriangle(10, 120, 40, 100, 40, 140, C_UI); // Left
  tft.fillTriangle(310, 120, 280, 100, 280, 140, C_UI); // Right
  tft.fillTriangle(160, 10, 140, 40, 180, 40, C_UI); // Up
  tft.fillTriangle(160, 230, 140, 200, 180, 200, C_UI); // Down
}

// Only draw the grid once to save FPS, or redraw if needed.
// For "Clipping" method, we can redraw it every frame without flicker.
void drawMinimapGrid() {
  for (int x = 0; x < MAP_WIDTH; x++) {
    for (int y = 0; y < MAP_HEIGHT; y++) {
      uint16_t color = (worldMap[x][y] > 0) ? C_MM_WALL : C_MM_BG;
      tft.fillRect(MM_OFFSET_X + x * MM_SCALE, MM_OFFSET_Y + y * MM_SCALE, MM_SCALE, MM_SCALE, color);
    }
  }
}

void drawMinimapPlayer() {
  // Just draw the player dot and line
  int px = MM_OFFSET_X + (int)(posX * MM_SCALE);
  int py = MM_OFFSET_Y + (int)(posY * MM_SCALE);
  tft.fillRect(px, py, 2, 2, C_UI); 
  tft.drawLine(px, py, px + (int)(dirX * 4), py + (int)(dirY * 4), C_UI);
}

void renderView() {
  tft.startWrite(); 

  // --- 3D RENDERING LOOP ---
  for(int x = 0; x < SCREEN_WIDTH; x += RES_DIVIDER) {
    
    // --- CLIPPING LOGIC ---
    // If the current ray is behind the minimap, we start drawing LOWER down the screen.
    // This creates a "safe zone" that the 3D engine cannot touch.
    int clipTop = 0;
    if (x < MM_SAFE_WIDTH) {
        clipTop = MM_SAFE_HEIGHT;
    }
    // ----------------------

    float cameraX = 2 * x / (float)SCREEN_WIDTH - 1; 
    float rayDirX = dirX + planeX * cameraX;
    float rayDirY = dirY + planeY * cameraX;

    int mapX = int(posX);
    int mapY = int(posY);

    float deltaDistX = (rayDirX == 0) ? 1e30 : fabsf(1.0f / rayDirX);
    float deltaDistY = (rayDirY == 0) ? 1e30 : fabsf(1.0f / rayDirY);

    float sideDistX, sideDistY, perpWallDist;
    int stepX, stepY, hit = 0, side;    

    if (rayDirX < 0) { stepX = -1; sideDistX = (posX - mapX) * deltaDistX; }
    else             { stepX = 1;  sideDistX = (mapX + 1.0f - posX) * deltaDistX; }
    if (rayDirY < 0) { stepY = -1; sideDistY = (posY - mapY) * deltaDistY; }
    else             { stepY = 1;  sideDistY = (mapY + 1.0f - posY) * deltaDistY; }

    while (hit == 0) {
      if (sideDistX < sideDistY) { sideDistX += deltaDistX; mapX += stepX; side = 0; }
      else                       { sideDistY += deltaDistY; mapY += stepY; side = 1; }
      if (worldMap[mapX][mapY] > 0) hit = 1;
    }

    if (side == 0) perpWallDist = (sideDistX - deltaDistX);
    else           perpWallDist = (sideDistY - deltaDistY);

    int lineHeight = (int)(SCREEN_HEIGHT / perpWallDist);

    // Calculate drawing bounds
    int drawStart = -lineHeight / 2 + SCREEN_HEIGHT / 2;
    if(drawStart < 0) drawStart = 0;
    int drawEnd = lineHeight / 2 + SCREEN_HEIGHT / 2;
    if(drawEnd >= SCREEN_HEIGHT) drawEnd = SCREEN_HEIGHT - 1;

    uint16_t color;
    switch(worldMap[mapX][mapY]) {
      case 1: color = C_WALL1; break; 
      case 2: color = C_WALL2; break;
      case 3: color = C_WALL3; break;
      case 4: color = C_WALL4; break;
      default: color = TFT_WHITE; break;
    }
    if (side == 1) color = (color >> 1) & 0x7BEF; 

    // --- RENDER WITH CLIPPING ---
    
    // 1. Draw Ceiling (Sky)
    // Only draw if the sky is visible below the clip zone
    if (drawStart > clipTop) {
        tft.fillRect(x, clipTop, RES_DIVIDER, drawStart - clipTop, C_SKY);
    }

    // 2. Draw Wall
    // We must handle the case where the wall is partially or fully behind the map
    int wallStart = drawStart;
    int wallHeight = drawEnd - drawStart + 1;
    
    if (wallStart < clipTop) {
        int diff = clipTop - wallStart;
        wallStart = clipTop;
        wallHeight -= diff;
    }
    
    if (wallHeight > 0) {
        tft.fillRect(x, wallStart, RES_DIVIDER, wallHeight, color);
    }

    // 3. Draw Floor
    // Floor is usually below the map, but we check just in case
    if (drawEnd < SCREEN_HEIGHT - 1) {
        int floorStart = drawEnd + 1;
        if (floorStart < clipTop) floorStart = clipTop;
        int floorH = SCREEN_HEIGHT - floorStart;
        if (floorH > 0) {
            tft.fillRect(x, floorStart, RES_DIVIDER, floorH, C_FLOOR);
        }
    }
  }

  // --- DRAW OVERLAYS ---
  // Now we draw the UI. Since the 3D engine didn't touch the map area,
  // there is no flickering!
  drawMinimapGrid();   
  drawMinimapPlayer();
  drawUI();
  
  tft.endWrite();
}

void handleInput() {
  if (touchscreen.tirqTouched() && touchscreen.touched()) {
    TS_Point p = touchscreen.getPoint();
    int tx = map(p.x, 200, 3700, 0, SCREEN_WIDTH);
    int ty = map(p.y, 240, 3800, 0, SCREEN_HEIGHT);

    float cosRot = cos(rotSpeed);
    float sinRot = sin(rotSpeed);
    float cosRotInv = cos(-rotSpeed);
    float sinRotInv = sin(-rotSpeed);

    // ZONE 1: LEFT (Rotate Left)
    if (tx < 80) { 
      float oldDirX = dirX;
      dirX = dirX * cosRot - dirY * sinRot;
      dirY = oldDirX * sinRot + dirY * cosRot;
      float oldPlaneX = planeX;
      planeX = planeX * cosRot - planeY * sinRot;
      planeY = oldPlaneX * sinRot + planeY * cosRot;
    }
    // ZONE 2: RIGHT (Rotate Right)
    else if (tx > 240) { 
      float oldDirX = dirX;
      dirX = dirX * cosRotInv - dirY * sinRotInv;
      dirY = oldDirX * sinRotInv + dirY * cosRotInv;
      float oldPlaneX = planeX;
      planeX = planeX * cosRotInv - planeY * sinRotInv;
      planeY = oldPlaneX * sinRotInv + planeY * cosRotInv;
    }
    // ZONE 3: CENTER TOP (Forward)
    else if (ty < 120) { 
      if(worldMap[int(posX + dirX * moveSpeed)][int(posY)] == 0) posX += dirX * moveSpeed;
      if(worldMap[int(posX)][int(posY + dirY * moveSpeed)] == 0) posY += dirY * moveSpeed;
    }
    // ZONE 4: CENTER BOTTOM (Backward)
    else { 
      if(worldMap[int(posX - dirX * moveSpeed)][int(posY)] == 0) posX -= dirX * moveSpeed;
      if(worldMap[int(posX)][int(posY - dirY * moveSpeed)] == 0) posY -= dirY * moveSpeed;
    }
  }
}