#pragma once
#include <Arduino.h>
#include <LittleFS.h>

// =========================================================================
// HARDWARE PIN DEFINITIONS (ESP32-C3 SuperMini)
// =========================================================================
#define PIN_TFT_SCL    6    // SPI Clock (SCL / SCK) - GPIO 6
#define PIN_TFT_SDA    10   // SPI MOSI (SDA / DIN) - GPIO 10
#define PIN_TFT_RES    5    // Display Reset (RES / RST) - GPIO 5
#define PIN_TFT_DC     3    // Data / Command (DC) - GPIO 3
#define PIN_TFT_CS     -1   // No CS pin on 7-pin display
#define PIN_TFT_BL     4    // Backlight Control (BLK) - GPIO 4

// TTP223 Touch Sensor (RTC GPIO 1)
#define PIN_TOUCH      1    // Touch signal pin

// Onboard Blue Debug LED (GPIO 8)
#define PIN_DEBUG_LED  8    // Onboard Blue LED

// Battery Voltage Sensing (GPIO 0 - ADC1_CH0)
#define PIN_BAT_ADC    0    // Center tap of 100k+100k voltage divider

// =========================================================================
// BLE UNIVERSAL 128-BIT VENDOR UUIDs
// =========================================================================
#define SERVICE_UUID           "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
#define CHAR_MODE_UUID         "6e400002-b5a3-f393-e0a9-e50e24dcca9e"
#define CHAR_PET_UUID          "6e400003-b5a3-f393-e0a9-e50e24dcca9e"
#define CHAR_TEXT_UUID         "6e400004-b5a3-f393-e0a9-e50e24dcca9e"
#define CHAR_TIME_UUID         "6e400005-b5a3-f393-e0a9-e50e24dcca9e"
#define CHAR_SETTINGS_UUID     "6e400006-b5a3-f393-e0a9-e50e24dcca9e"
#define CHAR_BATTERY_UUID      "6e400007-b5a3-f393-e0a9-e50e24dcca9e"
#define CHAR_STREAM_UUID       "6e400008-b5a3-f393-e0a9-e50e24dcca9e"

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
