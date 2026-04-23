//                            USER DEFINED SETTINGS
//   Set driver type, fonts to be loaded, pins used and SPI control method etc.
//
//   See the User_Setup.h file for details on how these settings can be changed.
//
//   This file is part of the TFT_eSPI library.

#ifndef USER_SETUP_H
#define USER_SETUP_H

// ##################################################################################
//
// Section 1: Display driver type (must be defined)
//
// ##################################################################################

#define ILI9341_DRIVER     // For the ILI9341 chip on your display

// ##################################################################################
//
// Section 2: Define the pins that are used for the parallel interface
//
// ##################################################################################

#define ESP32_PARALLEL      // Tell the library we are using parallel connection

// Control pins
#define TFT_CS   27         // Chip select
#define TFT_DC   14         // Data / Command (this is also the YP touch pin)
#define TFT_RST  26         // Reset (if connected to EN, set to -1)
#define TFT_WR   32         // Write strobe

// Parallel data bus pins D0..D7
#define TFT_D0   16         // (also XP touch pin)
#define TFT_D1    4         // (also XM touch pin)
#define TFT_D2   23
#define TFT_D3   22
#define TFT_D4   21
#define TFT_D5   19
#define TFT_D6   18
#define TFT_D7   17

// Backlight control (if used)
#define TFT_BL   -1         // -1 means not connected

// ##################################################################################
//
// Section 3: Touch screen configuration (4‑wire analog resistive)
//
// ##################################################################################

#define TOUCH_SCREEN_ANALOG   // Use the built‑in analog touch handler

// Touch control pins (these must match the display pins they are shared with)
#define TOUCH_YP 14           // Y+ (shared with TFT_DC)
#define TOUCH_XM  4           // X- (shared with TFT_D1)
#define TOUCH_YM 27           // Y- (shared with TFT_CS)
#define TOUCH_XP 16           // X+ (shared with TFT_D0)

// Optional: set the ADC resolution (the ESP32 default is 12 bits, which is fine)
// #define TOUCH_ADC_RESOLUTION 12

// Optional: set pressure thresholds (if needed, the library defaults are usually OK)
// #define TOUCH_MIN_PRESS 50
// #define TOUCH_MAX_PRESS 4000

// ##################################################################################
//
// Section 4: Fonts to be loaded (standard fonts are usually sufficient)
//
// ##################################################################################

#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF
#define SMOOTH_FONT

// ##################################################################################
//
// Section 5: Other optional settings (you can leave these as they are)
//
// ##################################################################################

// For ESP32, use the default SPIFFS for FS (not needed for parallel)
// #define FS_NO_GLOBALS

// Uncomment to use the SPIFFS for storing files (if you add an SD card later)
// #define USE_SPIFFS
#define TFT_MISO 25
#define TFT_MOSI 33
#define TFT_SCLK 2
#endif