#pragma once
#include <Arduino.h>
#include <LittleFS.h>

// =========================================================================
// HARDWARE PIN DEFINITIONS (ESP32-C3 SuperMini)
// =========================================================================

// 1.3" ST7789 240x240 SPI Display Pins
#define PIN_TFT_SCL    6    // SPI Clock (SCL / SCK) - GPIO 6
#define PIN_TFT_SDA    10   // SPI MOSI (SDA / DIN) - GPIO 10
#define PIN_TFT_RES    5    // Display Reset (RES / RST) - GPIO 5
#define PIN_TFT_DC     3    // Data / Command (DC) - GPIO 3
#define PIN_TFT_CS     -1   // No CS pin on 7-pin display
#define PIN_TFT_BL     2    // Backlight Control (BLK) - GPIO 2

// TTP223 Touch Sensor (RTC GPIO 1)
#define PIN_TOUCH      1    // Touch signal pin

// Onboard Blue Debug LED (GPIO 8)
#define PIN_DEBUG_LED  8    // Onboard Blue LED

// Battery Voltage Sensing (GPIO 0 - ADC1_CH0)
#define PIN_BAT_ADC    0    // Center tap of 100k+100k voltage divider

// =========================================================================
// BLE 16-BIT COMPACT UUIDs
// =========================================================================
#define SERVICE_UUID           "FFE0"
#define CHAR_MODE_UUID         "FFE1" // Mode Select
#define CHAR_PET_UUID          "FFE2" // Pet Avatar & Interaction
#define CHAR_TEXT_UUID         "FFE3" // Custom Text Message
#define CHAR_TIME_UUID         "FFE4" // Time & Weather Sync
#define CHAR_SETTINGS_UUID     "FFE5" // Brightness, Rotation, Sleep, Boot, Touch telemetry
#define CHAR_BATTERY_UUID      "FFE6" // Live Battery Voltage & Telemetry
#define CHAR_STREAM_UUID       "FFE7" // Live Custom Image & Video Stream Chunks

// =========================================================================
// SYSTEM MODES & BOOT SPLASH TYPES
// =========================================================================
enum SystemMode {
    MODE_CYBERPET = 0,   // Interactive Meme Emotion Mascot (Luffy, Shy, Cat, etc.)
    MODE_ROBOT_EYES = 1, // Animated procedural robot eyes
    MODE_CYBER_HUD = 2,  // Sci-Fi Clock & Temperature
    MODE_MATRIX_RAIN = 3,// Matrix Digital Rain
    MODE_TEXT_SCROLL = 4,// Screen-filling Max-Font Scrolling Message
    MODE_STREAM_MEDIA = 5// Live Web-BLE Custom Image / Video Stream
};

enum BootSplashType {
    BOOT_JOYBOY_INTRO = 0, // Built-in 2s Cyber Coffee Animation
    BOOT_CUSTOM_IMAGE = 1, // Stored /boot_splash.jpg from LittleFS
    BOOT_CUSTOM_ANIM  = 2, // Stored /boot_anim.bin from LittleFS
    BOOT_INSTANT      = 3  // Skip boot splash entirely
};

enum PetMood {
    MOOD_IDLE,
    MOOD_HAPPY,
    MOOD_ANGRY,
    MOOD_SLEEPING,
    MOOD_LOVE
};

enum PetAvatar {
    PET_CYBER_CAT = 0,
    PET_MECH_BOT = 1,
    PET_PIXEL_DRAGON = 2
};
