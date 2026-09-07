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

// --- arduino-esp32 3.x (ESP-IDF 5) compatibility fix for ESP32-C3 ------------
// TFT_eSPI's C3 back-end derives its raw SPI register pointers from
// REG_SPI_BASE(SPI_PORT) with SPI_PORT = SPI2_HOST (== 1). IDF 5's soc.h for
// the C3 defines REG_SPI_BASE(i) as ((i)==2 ? DR_REG_SPI2_BASE : 0), so that
// evaluates to 0 and the very first tft.init() write faults at address 0x10
// (SPI_USER_REG) -- "Guru Meditation Error: Store access fault, MTVAL 0x10".
// TFT_eSPI carries its own `#ifndef REG_SPI_BASE -> DR_REG_SPI2_BASE` fallback
// but it never fires because IDF already defines the macro. This file is
// included by TFT_eSPI.h after <Arduino.h>/soc.h and before the processor
// header, so forcing the fallback here fixes it without patching the library.
// The C3 has a single general-purpose SPI (GPSPI2), so this is always right.
#if defined(CONFIG_IDF_TARGET_ESP32C3)
  #include <soc/soc.h>
  #undef REG_SPI_BASE
  #define REG_SPI_BASE(i) DR_REG_SPI2_BASE
#endif

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
