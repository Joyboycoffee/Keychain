#pragma once
#include <Arduino.h>

// =========================================================================
// HARDWARE PIN DEFINITIONS (ESP32-C3 SuperMini)
// =========================================================================

// 1.3" ST7789 240x240 SPI Display Pins
#define PIN_TFT_SCL    8    // SPI Clock (SCL / SCK)
#define PIN_TFT_SDA    10   // SPI MOSI (SDA / DIN)
#define PIN_TFT_RES    5    // Display Reset (RES / RST)
#define PIN_TFT_DC     3    // Data / Command (DC)
#define PIN_TFT_CS     7    // Chip Select (CS) - Set to -1 if your board doesn't have CS
#define PIN_TFT_BL     2    // Backlight Control (BLK / LED) - PWM enabled

// TTP223 Touch Sensor (Connected to RTC GPIO for Deep Sleep Wakeup)
#define PIN_TOUCH      1    // TTP223 I/O Signal Pin (GPIO 0-5 support RTC wakeup)

// Battery Voltage Sensing (Optional ADC Divider on GPIO 0)
#define PIN_BAT_ADC    0    // 100k + 100k voltage divider to read 0-4.2V

// =========================================================================
// BLE SERVICE & CHARACTERISTIC UUIDs
// =========================================================================
#define SERVICE_UUID           "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHAR_MODE_UUID         "beb5483e-36e1-4688-b7f5-ea07361b26a8" // Mode Select
#define CHAR_PET_UUID          "1c95d5e3-d8f7-413a-bf3d-7a2e5d7be87e" // Pet Avatar & Interaction
#define CHAR_TEXT_UUID         "d8e4f1a2-5b3c-4e89-a1d2-9c8b7a6f5e4d" // Custom Text Message
#define CHAR_TIME_UUID         "e9a1b2c3-d4e5-6f7a-8b9c-0d1e2f3a4b5c" // Time & Weather Sync
#define CHAR_SETTINGS_UUID     "f1a2b3c4-d5e6-7a8b-9c0d-1e2f3a4b5c6d" // Brightness & Sleep Config

// =========================================================================
// SYSTEM MODES
// =========================================================================
enum SystemMode {
    MODE_CYBERPET = 0,   // Interactive Tamagotchi mascot
    MODE_ROBOT_EYES = 1, // Animated procedural robot eyes
    MODE_CYBER_HUD = 2,  // Sci-Fi Clock, Temperature & Telemetry
    MODE_MATRIX_RAIN = 3,// Matrix Digital Rain
    MODE_TEXT_SCROLL = 4 // Scrolling custom message
};

enum PetMood {
    MOOD_IDLE,
    MOOD_HAPPY,     // Triggered by holding / rubbing touch sensor
    MOOD_ANGRY,     // Triggered by rapid spam / multi-tapping
    MOOD_SLEEPING,  // Triggered after inactivity
    MOOD_LOVE       // Heart eyes / purring
};

enum PetAvatar {
    PET_CYBER_CAT = 0,
    PET_MECH_BOT = 1,
    PET_PIXEL_DRAGON = 2
};
