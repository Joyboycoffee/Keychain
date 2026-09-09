#include <Arduino.h>
#include <NimBLEDevice.h>
#include "config.h"
#include "display_setup.h"
#include "robot_eyes.h"
#include "pet_engine.h"
#include "cyber_hud.h"
#include "matrix_rain.h"

// Instantiate Display & Double Buffer Sprite
LGFX_ST7789 tft;
LGFX_Sprite canvas(&tft);

// Mode Engines
SystemMode currentMode = MODE_CYBERPET;
RobotEyes robotEyes;
CyberPet cyberPet;
CyberHUD cyberHUD;
MatrixRain matrixRain;

// Custom Scrolling Text
String customMessage = "I am Joy Boy Coffee";
int scrollX = 240;

// Power & Settings
uint8_t screenBrightness = 220; // 0-255 PWM
uint32_t sleepTimeoutMs = 300000; // 5 min auto-sleep (prevents USB disconnection during dev)
uint32_t lastActivityTime = 0;
bool bleConnected = false;

// Battery Telemetry
float currentBatVoltage = 0.0f;
int currentBatPercent = 0;
uint32_t currentRawMv = 0;
uint32_t lastBatteryReadTime = 0;

// Touch Gesture Tracker
uint32_t touchStartTime = 0;
uint32_t touchReleaseTime = 0;
int tapCount = 0;
bool isTouching = false;

// Characteristic pointers
NimBLECharacteristic* pCharBattery = nullptr;

// =========================================================================
// BATTERY SENSING FILTER & DISCHARGE CURVE (64-SAMPLE MOVING AVERAGE)
// =========================================================================
int calculateLiPoPercent(float v) {
    if (v >= 4.20f) return 100;
    if (v <= 3.30f) return 0;
    if (v >= 4.05f) return 85 + (int)((v - 4.05f) / 0.15f * 15.0f);
    if (v >= 3.85f) return 55 + (int)((v - 3.85f) / 0.20f * 30.0f);
    if (v >= 3.70f) return 20 + (int)((v - 3.70f) / 0.15f * 35.0f);
    return (int)((v - 3.30f) / 0.40f * 20.0f);
}

void updateBatteryTelemetry() {
    uint32_t sumMv = 0;
    const int SAMPLES = 64;
    for (int i = 0; i < SAMPLES; i++) {
        sumMv += analogReadMilliVolts(PIN_BAT_ADC);
        delayMicroseconds(100);
    }
    currentRawMv = sumMv / SAMPLES;

    // 100k + 100k (or 200k + 200k) divider -> Multiply by 2.0
    currentBatVoltage = (currentRawMv * 2.0f) / 1000.0f;

    // If voltage < 2.5V (e.g. floating / USB diode leak when battery not active)
    if (currentBatVoltage < 2.5f) {
        currentBatPercent = 0;
    } else {
        currentBatPercent = calculateLiPoPercent(currentBatVoltage);
    }

    // Update HUD
    cyberHUD.setBattery((float)currentBatPercent, currentBatVoltage);

    // Update BLE Characteristic string (Format: "VOLTS|PCT|RAWMV" e.g. "3.85|70|1925")
    if (pCharBattery != nullptr) {
        char batPayload[32];
        if (currentBatVoltage < 2.5f) {
            snprintf(batPayload, sizeof(batPayload), "USB|100|%u", currentRawMv);
        } else {
            snprintf(batPayload, sizeof(batPayload), "%.2f|%d|%u", currentBatVoltage, currentBatPercent, currentRawMv);
        }
        pCharBattery->setValue(batPayload);
        if (bleConnected) {
            pCharBattery->notify();
        }
    }
}

// =========================================================================
// BLE CALLBACKS
// =========================================================================
class ServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer) {
        bleConnected = true;
        lastActivityTime = millis();
        Serial.println("[BLE] Client Connected!");
    }
    void onDisconnect(NimBLEServer* pServer) {
        bleConnected = false;
        lastActivityTime = millis();
        Serial.println("[BLE] Client Disconnected. Restarting Advertising...");
        NimBLEDevice::startAdvertising();
    }
};

class ModeCallback : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pChar) {
        std::string val = pChar->getValue();
        if (val.length() > 0) {
            int m = val[0] - '0';
            if (m >= 0 && m <= 4) {
                currentMode = (SystemMode)m;
                lastActivityTime = millis();
                Serial.printf("[BLE] Mode switched to: %d\n", m);
            }
        }
    }
};

class PetCallback : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pChar) {
        std::string val = pChar->getValue();
        if (val.length() > 0) {
            int a = val[0] - '0';
            if (a >= 0 && a <= 2) {
                cyberPet.setAvatar((PetAvatar)a);
                lastActivityTime = millis();
                Serial.printf("[BLE] Avatar switched to: %d\n", a);
            }
        }
    }
};

class TextCallback : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pChar) {
        std::string val = pChar->getValue();
        if (val.length() > 0) {
            customMessage = String(val.c_str());
            scrollX = 240;
            currentMode = MODE_TEXT_SCROLL;
            lastActivityTime = millis();
            Serial.printf("[BLE] Custom message received: %s\n", customMessage.c_str());
        }
    }
};

class TimeCallback : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pChar) {
        std::string val = pChar->getValue();
        // Format expected: "HH:MM:SS|TEMP" e.g. "21:45:00|27.5"
        if (val.length() >= 8) {
            int h = atoi(val.substr(0, 2).c_str());
            int m = atoi(val.substr(3, 2).c_str());
            int s = atoi(val.substr(6, 2).c_str());
            cyberHUD.setTime(h, m, s);
            
            size_t barIdx = val.find('|');
            if (barIdx != std::string::npos) {
                float temp = atof(val.substr(barIdx + 1).c_str());
                cyberHUD.setWeather(temp, "PHONE_SYNC");
            }
            lastActivityTime = millis();
            Serial.printf("[BLE] Synced Time: %02d:%02d:%02d\n", h, m, s);
        }
    }
};

class SettingsCallback : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pChar) {
        std::string val = pChar->getValue();
        if (val.length() > 0) {
            int br = atoi(val.c_str());
            if (br >= 10 && br <= 255) {
                screenBrightness = br;
                tft.setBrightness(screenBrightness);
                lastActivityTime = millis();
                Serial.printf("[BLE] Brightness set to: %d\n", br);
            }
        }
    }
};

// =========================================================================
// TOUCH GESTURES (RUB = HAPPY/LOVE, SPAM = ANGRY, TAP = NEXT MODE)
// =========================================================================
void processTouch() {
    bool rawTouch = digitalRead(PIN_TOUCH) == HIGH;
    uint32_t now = millis();

    if (rawTouch && !isTouching) {
        // Touch pressed
        isTouching = true;
        touchStartTime = now;
        lastActivityTime = now;
    } else if (!rawTouch && isTouching) {
        // Touch released
        isTouching = false;
        touchReleaseTime = now;
        uint32_t duration = touchReleaseTime - touchStartTime;

        if (duration < 350) {
            tapCount++;
        }
    }

    // Check for Continuous Rub / Long Press (> 1.0 second)
    if (isTouching && (now - touchStartTime > 1000)) {
        cyberPet.setMood(MOOD_LOVE);
        robotEyes.setMood(MOOD_LOVE);
        lastActivityTime = now;
    }

    // Check for Multi-tap timeout window
    if (!isTouching && tapCount > 0 && (now - touchReleaseTime > 350)) {
        if (tapCount >= 3) {
            // Rage Tapped -> ANGRY MODE!
            cyberPet.setMood(MOOD_ANGRY);
            robotEyes.setMood(MOOD_ANGRY);
        } else if (tapCount == 1) {
            // Single tap -> Cycle mode
            if (currentMode == MODE_CYBERPET) {
                // Cycle avatar inside pet mode first, or tap to next mode
                currentMode = MODE_ROBOT_EYES;
            } else if (currentMode == MODE_ROBOT_EYES) {
                currentMode = MODE_CYBER_HUD;
            } else if (currentMode == MODE_CYBER_HUD) {
                currentMode = MODE_MATRIX_RAIN;
            } else if (currentMode == MODE_MATRIX_RAIN) {
                currentMode = MODE_TEXT_SCROLL;
            } else {
                currentMode = MODE_CYBERPET;
                cyberPet.setAvatar((PetAvatar)((cyberPet.currentAvatar + 1) % 3));
            }
        }
        tapCount = 0;
        lastActivityTime = now;
    }
}

// =========================================================================
// DEEP SLEEP ROUTINE
// =========================================================================
void enterDeepSleep() {
    Serial.println("[POWER] Entering Deep Sleep. Wakeup on Touch GPIO 1...");
    // Fade screen backlight out smoothly
    for (int b = screenBrightness; b >= 0; b -= 20) {
        tft.setBrightness(b);
        delay(15);
    }
    tft.writeCommand(0x10); // ST7789 Sleep In command

    // Configure ESP32-C3 RTC GPIO Wakeup on Touch Pin (GPIO 1)
    gpio_wakeup_enable((gpio_num_t)PIN_TOUCH, GPIO_INTR_HIGH_LEVEL);
    esp_deep_sleep_enable_gpio_wakeup(1ULL << PIN_TOUCH, ESP_GPIO_WAKEUP_GPIO_HIGH);

    esp_deep_sleep_start();
}

// =========================================================================
// SETUP
// =========================================================================
void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n\n========================================");
    Serial.println("  ESP32-C3 CYBER KEYCHAIN v2.0 READY");
    Serial.println("========================================");

    // Hardware Pins
    pinMode(PIN_DEBUG_LED, OUTPUT);
    pinMode(PIN_TOUCH, INPUT);
    analogSetPinAttenuation(PIN_BAT_ADC, ADC_11db);
    pinMode(PIN_BAT_ADC, INPUT);

    // Display init
    tft.init();
    tft.setRotation(1); // Rotated 90 degrees clockwise to the right
    tft.setBrightness(screenBrightness);

    // Sprite Double Buffer
    canvas.setColorDepth(16);
    canvas.createSprite(240, 240);

    // Initial Battery Reading
    updateBatteryTelemetry();

    // Initialize NimBLE Bluetooth Server
    NimBLEDevice::init("CYBER_KEYCHAIN");
    NimBLEDevice::setPower(ESP_PWR_LVL_P9); // Max TX power
    NimBLEServer* pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());

    NimBLEService* pService = pServer->createService(SERVICE_UUID);

    auto pCharMode = pService->createCharacteristic(CHAR_MODE_UUID, NIMBLE_PROPERTY::WRITE);
    pCharMode->setCallbacks(new ModeCallback());

    auto pCharPet = pService->createCharacteristic(CHAR_PET_UUID, NIMBLE_PROPERTY::WRITE);
    pCharPet->setCallbacks(new PetCallback());

    auto pCharText = pService->createCharacteristic(CHAR_TEXT_UUID, NIMBLE_PROPERTY::WRITE);
    pCharText->setCallbacks(new TextCallback());

    auto pCharTime = pService->createCharacteristic(CHAR_TIME_UUID, NIMBLE_PROPERTY::WRITE);
    pCharTime->setCallbacks(new TimeCallback());

    auto pCharSet = pService->createCharacteristic(CHAR_SETTINGS_UUID, NIMBLE_PROPERTY::WRITE);
    pCharSet->setCallbacks(new SettingsCallback());

    pCharBattery = pService->createCharacteristic(CHAR_BATTERY_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);

    pService->start();

    NimBLEAdvertising* pAdv = NimBLEDevice::getAdvertising();
    pAdv->addServiceUUID(SERVICE_UUID);
    pAdv->start();

    lastActivityTime = millis();
    Serial.println("[BLE] Advertising started as CYBER_KEYCHAIN (0xFFE0)");
}

// =========================================================================
// MAIN LOOP
// =========================================================================
void loop() {
    // 1. Process Touch Gestures
    processTouch();

    // 2. Periodic Battery Sampling (every 2 seconds)
    if (millis() - lastBatteryReadTime > 2000) {
        lastBatteryReadTime = millis();
        updateBatteryTelemetry();
    }

    // 3. Heartbeat LED (blinks slowly when running)
    static uint32_t lastBlink = 0;
    static bool ledState = false;
    if (millis() - lastBlink > 500) {
        lastBlink = millis();
        ledState = !ledState;
        digitalWrite(PIN_DEBUG_LED, ledState ? HIGH : LOW);
    }

    // 4. Render Active Mode to Double Buffer Sprite
    switch (currentMode) {
        case MODE_CYBERPET:
            cyberPet.update();
            break;

        case MODE_ROBOT_EYES:
            robotEyes.update();
            break;

        case MODE_CYBER_HUD:
            cyberHUD.update();
            break;

        case MODE_MATRIX_RAIN:
            matrixRain.update();
            break;

        case MODE_TEXT_SCROLL:
            canvas.fillScreen(TFT_BLACK);
            // Cyber Frame
            canvas.drawRoundRect(6, 6, 228, 228, 8, 0x07FF);
            canvas.setTextColor(0x07FF, TFT_BLACK);
            canvas.setTextSize(1);
            canvas.drawString("NEURAL BANNER //", 16, 16);

            // Scrolling Marquee
            canvas.fillRoundRect(8, 90, 224, 60, 6, 0x0821);
            canvas.drawRoundRect(8, 90, 224, 60, 6, 0x07E0);
            canvas.setTextColor(0x07E0, 0x0821);
            canvas.setTextSize(2);
            canvas.drawString(customMessage, scrollX, 112);

            scrollX -= 4;
            if (scrollX < -((int)customMessage.length() * 18)) {
                scrollX = 240;
            }

            // Battery footer
            canvas.setTextColor(0xFFE0, TFT_BLACK);
            canvas.setTextSize(1);
            char bStr[36];
            if (currentBatVoltage < 2.5f) {
                snprintf(bStr, sizeof(bStr), "PWR: USB-C (5V) | %umV", currentRawMv);
            } else {
                snprintf(bStr, sizeof(bStr), "BAT: %.2fV [%d%%]", currentBatVoltage, currentBatPercent);
            }
            canvas.drawCenterString(bStr, 120, 195);
            break;
    }

    // 5. Push Frame to Display (Hardware DMA Transfer)
    canvas.pushSprite(0, 0);

    // 6. Deep Sleep if Unplugged, Idle, and No BLE Connection
    if (!bleConnected && (millis() - lastActivityTime > sleepTimeoutMs)) {
        enterDeepSleep();
    }

    delay(10);
}

