// CyberSaiyan ESP32-C3 badge setup for TFT_eSPI
// Covers the WHY2025/EMF2026 badge (github.com/CyberSaiyanIT/why2025-badge) and
// the RHC22 badge (github.com/CyberSaiyanIT/rhc22-badge): both share the same
// ESP32-C3 + ST7789 240x320 panel wiring, so this one file serves either.
//
// Copy this file to:
//   <Arduino/libraries>/TFT_eSPI/User_Setup.h
// (replacing whichever User_Setup.h is there for another board profile) and
// keep the stock User_Setup_Select.h (or restore its backup) so TFT_eSPI can
// still load its driver command definitions correctly.
//
// Build the sketch with board "ESP32C3 Dev Module" and the preprocessor
// define AQUARIUM_BOARD_CYBERSAIYAN_C3 set (Arduino IDE: Sketch > ... > select
// this board, then add the define via Tools > "Extra Flags", or edit the
// sketch's kBoardProfileName block directly).

#define USER_SETUP_INFO "CyberSaiyan ESP32-C3 badge ST7789 240x320"
#define USER_SETUP_ID 71

// Display driver
#define ST7789_DRIVER
#define TFT_RGB_ORDER TFT_RGB  // If colours look swapped on real hardware, try TFT_BGR instead.
// Some ST7789 panels need inverted colours -- uncomment if colours look negative:
// #define TFT_INVERSION_ON

// ESP32-C3 TFT pin mapping (WHY2025/EMF2026 and RHC22 badge wiring)
#define TFT_MOSI 7
#define TFT_MISO 2
#define TFT_SCLK 6
#define TFT_CS   10
#define TFT_DC   4
#define TFT_RST  3
// No TFT_BL here: backlight is behind an AW9523 I2C GPIO expander, not a
// plain GPIO -- the sketch drives it directly for the CyberSaiyan board
// profile (see initCyberSaiyanBacklight() in the .ino).

// Display driver's native geometry (the sketch rotates this to a fixed
// 320x240 landscape render surface at runtime via displayRotation()).
#define TFT_WIDTH  240
#define TFT_HEIGHT 320

#define SPI_FREQUENCY       40000000
#define SPI_READ_FREQUENCY  20000000
// No touch controller wired to the MCU on this badge -- SPI_TOUCH_FREQUENCY
// is unused (initTouchHardware() returns false for this board profile).

// Fonts
#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF
#define SMOOTH_FONT
