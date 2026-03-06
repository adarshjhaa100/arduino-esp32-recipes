/* MINI OS GUI Demo for ESP32 CYD
   This file implements a simple 2D touch-based Graphical User Interface (GUI)
   with multiple screens (Desktop, Clock, Info, Calculator) to simulate a minimal OS shell.
   
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

// --- GUI SETTINGS ---
#define SCREEN_W 320
#define SCREEN_H 240
#define FONT_SIZE 2 // Default font size for text

// Colors (repurposed from selected code)
#define C_BLACK    0x0000 // Background / Text
#define C_WHITE    0xFFFF // Highlight / Text
#define C_SKY      0x3186 // Header Bar Color
#define C_FLOOR    0x3183 // Desktop Background Color
#define C_WALL_1   0xB800 // App Button 1 (Red)
#define C_WALL_2   0x8000 // App Button 2 (Dark Red)
#define C_GUN      0x52AA // App Button 3 (Grey/Blue)
#define C_CALC_KEY 0x9492 // Calculator Key Color (Dark Gray)
#define C_CALC_OP  0xD66E // Calculator Operator Color (Orange)
#define C_CALC_DIS 0x3D11 // Calculator Display Background

// --- OS STATE MANAGEMENT ---
enum ScreenState {
  DESKTOP,
  CLOCK_APP,
  INFO_APP,
  CALCULATOR_APP // <-- New Calculator State
};

ScreenState currentScreen = DESKTOP;
long startTime = 0; // For clock calculation

// --- CALCULATOR STATE & CONSTANTS ---
char calc_display[16] = "0"; // Display text buffer
double calc_result = 0;
double calc_operand = 0;
char calc_operation = '\0'; // '+', '-', '*', '/', '='
bool calc_clear_display = true; // Flag to clear display when starting new number
bool calc_error = false; // Error flag for display

// Constants for Calculator Layout (Ensures draw and touch use identical dimensions)
const int CALC_START_X = 10;
const int CALC_START_Y = 90;
const int CALC_SPACING = 5;
// Calculated width: (320 - 20 - 15) / 4 = 71
const int CALC_BTN_W = (SCREEN_W - (CALC_START_X * 2) - (CALC_SPACING * 3)) / 4;
// Calculated height: (240 - 90 - 15 - 5) / 4 = 32
const int CALC_BTN_H = (SCREEN_H - CALC_START_Y - (CALC_SPACING * 4)) / 4;


// Structure to define App Buttons
struct Button {
  int x, y, w, h;
  uint16_t color;
  const char* label;
  ScreenState targetState;
};

// Define the buttons for the DESKTOP screen
Button appButtons[] = {
  {30, 60, 80, 80, C_WALL_1, "CLOCK", CLOCK_APP},
  {120, 60, 80, 80, C_WALL_2, "INFO", INFO_APP},
  {210, 60, 80, 80, C_GUN, "CALC", CALCULATOR_APP} // <-- Calculator App
};
const int NUM_BUTTONS = sizeof(appButtons) / sizeof(appButtons[0]);


// --- DRAWING FUNCTIONS ---

void drawHeader(const char* title) {
  tft.fillRect(0, 0, SCREEN_W, 30, C_SKY);
  tft.setTextColor(C_WHITE, C_SKY);
  tft.drawCentreString(title, SCREEN_W / 2, 10, FONT_SIZE);
  
  // Draw Home Button (Back arrow)
  tft.fillRect(0, 0, 30, 30, C_SKY);
  tft.drawRect(5, 5, 20, 20, C_WHITE);
  tft.fillTriangle(20, 15, 10, 10, 10, 20, C_WHITE); // Simple arrow
}

void drawDesktop() {
  tft.fillScreen(C_FLOOR); // Desktop background
  drawHeader("MINI-OS v1.0");

  tft.setTextColor(C_WHITE, C_FLOOR);
  tft.drawCentreString("Touch an App to Launch", SCREEN_W / 2, 40, FONT_SIZE);

  for (int i = 0; i < NUM_BUTTONS; i++) {
    Button btn = appButtons[i];
    // Draw Button Box
    tft.fillRect(btn.x, btn.y, btn.w, btn.h, btn.color);
    tft.drawRect(btn.x, btn.y, btn.w, btn.h, C_BLACK);
    
    // Draw Label
    tft.setTextColor(C_WHITE, btn.color);
    tft.drawCentreString(btn.label, btn.x + btn.w / 2, btn.y + btn.h / 2 - 5, FONT_SIZE);
  }
}

void drawClockApp() {
  tft.fillScreen(C_BLACK);
  drawHeader("CLOCK");

  // Calculate elapsed time (simulated time)
  unsigned long elapsedMillis = millis() - startTime;
  int seconds = elapsedMillis / 1000;
  int minutes = seconds / 60;
  int hours = minutes / 60;
  
  seconds %= 60;
  minutes %= 60;

  // Format time string
  char timeStr[10];
  sprintf(timeStr, "%02d:%02d:%02d", hours % 24, minutes, seconds);

  // Draw Time
  tft.setTextColor(C_WHITE, C_BLACK);
  tft.setTextSize(4); // Large font for time
  tft.drawCentreString(timeStr, SCREEN_W / 2, SCREEN_H / 2 - 20, 1); // Font 1 is used with setTextSize
  
  tft.setTextSize(1); // Reset font size
  tft.drawCentreString("SYSTEM TIME (SIMULATED)", SCREEN_W / 2, SCREEN_H / 2 + 30, FONT_SIZE);
}

void drawInfoApp() {
  tft.fillScreen(C_BLACK);
  drawHeader("SYSTEM INFO");
  
  tft.setTextColor(C_WHITE, C_BLACK);
  int y = 50;
  
  tft.drawString("Platform: ESP32 CYD", 20, y, FONT_SIZE);
  y += 20;
  tft.drawString("Display: 320x240 TFT", 20, y, FONT_SIZE);
  y += 20;
  tft.drawString("Kernel: Arduino Core", 20, y, FONT_SIZE);
  y += 20;
  tft.drawString("CPU Freq: 240 MHz (Typ.)", 20, y, FONT_SIZE);
  y += 20;
  tft.drawString("Touch: XPT2046", 20, y, FONT_SIZE);
  y += 40;
  tft.drawCentreString("A simple GUI environment.", SCREEN_W / 2, y, FONT_SIZE);
}

// --- CALCULATOR IMPLEMENTATION ---

// Button definitions: 4 rows, 4 columns
const char* calc_keys[4][4] = {
    {"C", "/", "*", "-"},
    {"7", "8", "9", "+"},
    {"4", "5", "6", "="},
    {"1", "2", "3", "."}
};

void drawCalcButton(int x, int y, int w, int h, const char* label) {
  uint16_t color = C_CALC_KEY;
  if (label[0] == '/' || label[0] == '*' || label[0] == '-' || label[0] == '+' || label[0] == '=') {
    color = C_CALC_OP;
  }
  if (label[0] == 'C') {
    color = C_WALL_1; // Red for Clear
  }
  
  tft.fillRect(x + 2, y + 2, w - 4, h - 4, color);
  tft.drawRect(x, y, w, h, C_BLACK);
  tft.setTextColor(C_WHITE, color);
  tft.drawCentreString(label, x + w / 2, y + h / 2 - 5, FONT_SIZE);
}

void drawCalculatorApp() {
  tft.fillScreen(C_BLACK);
  drawHeader("CALCULATOR");
  
  // Display Area
  tft.fillRect(CALC_START_X, 40, SCREEN_W - (CALC_START_X * 2), 40, C_CALC_DIS);
  tft.drawRect(CALC_START_X, 40, SCREEN_W - (CALC_START_X * 2), 40, C_WHITE);

  tft.setTextColor(C_WHITE, C_CALC_DIS);
  tft.setTextDatum(TR_DATUM); // Top Right alignment
  tft.drawRightString(calc_display, SCREEN_W - CALC_START_X - 5, 50, 4); // Draw display text
  tft.setTextDatum(TL_DATUM); // Reset to Top Left

  // Button Grid (4x4)
  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      int x = CALC_START_X + c * (CALC_BTN_W + CALC_SPACING);
      int y = CALC_START_Y + r * (CALC_BTN_H + CALC_SPACING);
      drawCalcButton(x, y, CALC_BTN_W, CALC_BTN_H, calc_keys[r][c]);
    }
  }
}

void calc_perform_operation() {
  double current_value = atof(calc_display);
  
  if (calc_operation == '+') calc_result += current_value;
  else if (calc_operation == '-') calc_result -= current_value;
  else if (calc_operation == '*') calc_result *= current_value;
  else if (calc_operation == '/') {
    if (current_value == 0) {
      calc_error = true;
      strcpy(calc_display, "Error");
      return;
    }
    calc_result /= current_value;
  }
  
  // Convert result back to string display (using 4 decimal places max for TFT)
  char buffer[16];
  dtostrf(calc_result, 1, 4, buffer);
  // Remove trailing zeros and decimal point if possible
  int len = strlen(buffer);
  while (len > 0 && buffer[len-1] == '0' && strchr(buffer, '.') != NULL) {
    buffer[len-1] = '\0';
    len--;
  }
  if (len > 0 && buffer[len-1] == '.') {
    buffer[len-1] = '\0';
  }
  strcpy(calc_display, buffer);
}

void calc_process_key(char key) {
  if (calc_error) {
    // If in error state, only 'C' can clear it
    if (key == 'C') {
      calc_error = false;
    } else {
      return;
    }
  }
  
  if (isdigit(key) || key == '.') {
    if (calc_clear_display) {
      strcpy(calc_display, (key == '.') ? "0." : &key);
      if (key != '.') calc_display[1] = '\0'; // Ensure only the single digit is present
      calc_clear_display = false;
    } else {
      if (strlen(calc_display) < 10) { // Limit length
        if (key == '.' && strchr(calc_display, '.') != NULL) return; // Only one decimal
        strncat(calc_display, &key, 1);
      }
    }
  } else if (key == 'C') {
    strcpy(calc_display, "0");
    calc_result = 0;
    calc_operand = 0;
    calc_operation = '\0';
    calc_clear_display = true;
  } else if (key == '+' || key == '-' || key == '*' || key == '/') {
    if (calc_operation != '\0' && !calc_clear_display) {
      calc_perform_operation();
      // The result is now in calc_display
    } else {
      calc_result = atof(calc_display);
    }
    calc_operation = key;
    calc_clear_display = true;
  } else if (key == '=') {
    if (calc_operation != '\0') {
      calc_perform_operation();
      calc_operation = '\0'; // Operation complete
      calc_clear_display = true;
    }
  }
}

void handleCalculatorInput(int tx, int ty) {
  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      int x = CALC_START_X + c * (CALC_BTN_W + CALC_SPACING);
      int y = CALC_START_Y + r * (CALC_BTN_H + CALC_SPACING);
      
      if (tx >= x && tx <= (x + CALC_BTN_W) && ty >= y && ty <= (y + CALC_BTN_H)) {
        // Found the key pressed
        const char* key_label = calc_keys[r][c];
        calc_process_key(key_label[0]);
        // Redraw to show the new number/result
        drawCalculatorApp(); 
        break;
      }
    }
  }
}

// --- DEBUG FUNCTION ---
void drawTouchCoordinates(int tx, int ty) {
  tft.setTextDatum(BR_DATUM); // Bottom Right alignment
  tft.setTextSize(1);
  
  // Clear old coordinates area (Moved up by 20 pixels)
  // New Y start: SCREEN_H - 35 (240 - 35 = 205)
  tft.fillRect(SCREEN_W - 85, SCREEN_H - 35, 85, 15, C_BLACK);
  
  char coordStr[20];
  sprintf(coordStr, "X:%d Y:%d", tx, ty);
  
  tft.setTextColor(C_WHITE, C_BLACK); // White text on Black background
  // New Y baseline: SCREEN_H - 25 (240 - 25 = 215)
  tft.drawRightString(coordStr, SCREEN_W - 5, SCREEN_H - 25, FONT_SIZE);
  
  tft.setTextDatum(TL_DATUM); // Reset to Top Left
}

// --- INPUT HANDLING ---

void handleTouch(int tx, int ty) {
  // Check for the Home Button (Back to Desktop)
  if (currentScreen != DESKTOP && tx >= 0 && tx <= 30 && ty >= 0 && ty <= 30) {
    currentScreen = DESKTOP;
    drawDesktop(); // <-- IMMEDIATE REDRAW on return to desktop
    return;
  }
  
  if (currentScreen == DESKTOP) {
    for (int i = 0; i < NUM_BUTTONS; i++) {
      Button btn = appButtons[i];
      if (tx >= btn.x && tx <= (btn.x + btn.w) && ty >= btn.y && ty <= (btn.y + btn.h)) {
        currentScreen = btn.targetState;
        
        // Handle App Initialization and immediate redraw on Launch
        if (currentScreen == CLOCK_APP) {
          startTime = millis();
          drawClockApp(); // <-- IMMEDIATE REDRAW on launch
        } else if (currentScreen == INFO_APP) {
          drawInfoApp(); // <-- IMMEDIATE REDRAW on launch
        } else if (currentScreen == CALCULATOR_APP) {
          // Initialize Calculator state
          strcpy(calc_display, "0");
          calc_result = 0;
          calc_operand = 0;
          calc_operation = '\0';
          calc_clear_display = true;
          calc_error = false;
          drawCalculatorApp(); // <-- IMMEDIATE REDRAW on launch
        }
        break;
      }
    }
  } else if (currentScreen == CALCULATOR_APP) {
    handleCalculatorInput(tx, ty);
  }
  // No specific touch handling needed for CLOCK_APP or INFO_APP yet, they are passive views.
}

void handleInput() {
  if (touchscreen.touched()) {
    TS_Point p = touchscreen.getPoint();
    
    // --- FIX: Swap raw X/Y and invert mapping ranges ---
    // Raw p.y (vertical) is mapped to tx (screen X).
    // The range is inverted (SCREEN_W to 1) to correct the X-axis inversion.
    int tx = map(p.y, 240, 3800, SCREEN_W, 1); 
    
    // Raw p.x (horizontal) is mapped to ty (screen Y).
    // The range is inverted (SCREEN_H to 1) to correct the Y-axis inversion.
    int ty = map(p.x, 200, 3700, SCREEN_H, 1); 
    
    handleTouch(tx, ty);
    
    // Draw coordinates on every touch
    drawTouchCoordinates(tx, ty);
    
    // Debounce delay
    delay(200);
  }
}

// --- MAIN FUNCTIONS ---

void setup() {
  Serial.begin(115200);

  // Init Touch
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  touchscreen.setRotation(1);

  // Init TFT
  tft.init();
  tft.setRotation(1);
  tft.setTextFont(2); // Use a simple built-in font for the GUI

  // Set initial screen state
  currentScreen = DESKTOP;
  
  // Draw the initial screen
  drawDesktop();
  // Clear coordinate display initially
  drawTouchCoordinates(0, 0); 
}

void loop() {
  handleInput();
  
  // Redraw the current screen only if it requires updating (like a clock)
  // NOTE: Static screens (DESKTOP, INFO_APP, CALCULATOR_APP) rely on explicit redraws in handleTouch()
  switch (currentScreen) {
    case DESKTOP:
      // Desktop is static
      break;
    case CLOCK_APP:
      drawClockApp(); // Needs constant updating
      delay(500);     // Half-second delay for the clock display
      break;
    case INFO_APP:
      // Info app is static
      break;
    case CALCULATOR_APP:
      // Calculator is mostly static, input handler redraws on touch
      break;
  }
}