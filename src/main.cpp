#include <Arduino.h>
#include <NimBLEDevice.h>
#include "config.h"
#include "display_setup.h"
#include "meme_pet.h"
#include "robot_eyes.h"
#include "cyber_hud.h"
#include "matrix_rain.h"

// Instantiate Display & Double Buffer Sprite
LGFX_ST7789 tft;
LGFX_Sprite canvas(&tft);

// Mode Engines
SystemMode currentMode = MODE_CYBERPET;
MemePet memePet;
RobotEyes robotEyes;
CyberHUD cyberHUD;
MatrixRain matrixRain;

// Custom Scrolling Text
String customMessage = "I am Joy Boy Coffee";
int scrollX = 240;

// Power & Settings
uint8_t screenBrightness = 220; // 0-255 PWM
uint32_t sleepTimeoutMs = 300000; // 5 min auto-sleep
uint32_t lastActivityTime = 0;
bool bleConnected = false;

// Touch Gesture Tracker
uint32_t touchStartTime = 0;
uint32_t touchReleaseTime = 0;
int tapCount = 0;
bool isTouching = false;

// Media Stream Buffer (for live custom images & video streaming over BLE)
#define STREAM_BUFFER_SIZE 32768
uint8_t streamBuffer[STREAM_BUFFER_SIZE];
size_t streamBytesReceived = 0;
size_t expectedStreamBytes = 0;
bool newMediaFrameReady = false;

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
            if (m >= 0 && m <= 5) {
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
            if (a >= 0 && a <= 4) {
                memePet.setEmotion((MemeEmotion)a);
                currentMode = MODE_CYBERPET;
                lastActivityTime = millis();
                Serial.printf("[BLE] Emotion switched to: %d\n", a);
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

class StreamCallback : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pChar) {
        std::string data = pChar->getValue();
        if (data.length() == 0) return;

        const uint8_t* bytes = (const uint8_t*)data.data();
        size_t len = data.length();

        // Header Packet: 0xAA 0x55 [size_high] [size_low]
        if (len >= 4 && bytes[0] == 0xAA && bytes[1] == 0x55) {
            expectedStreamBytes = (bytes[2] << 8) | bytes[3];
            streamBytesReceived = 0;
            if (len > 4) {
                size_t payload = len - 4;
                if (payload <= STREAM_BUFFER_SIZE) {
                    memcpy(streamBuffer, bytes + 4, payload);
                    streamBytesReceived = payload;
                }
            }
        } else {
            // Append payload chunk
            if (streamBytesReceived + len <= STREAM_BUFFER_SIZE) {
                memcpy(streamBuffer + streamBytesReceived, bytes, len);
                streamBytesReceived += len;
            }
        }

        if (streamBytesReceived >= expectedStreamBytes && expectedStreamBytes > 0) {
            newMediaFrameReady = true;
            currentMode = MODE_STREAM_MEDIA;
            lastActivityTime = millis();
        }
    }
};

// =========================================================================
// TOUCH GESTURES (SENSITIVE & INTUITIVE)
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

        if (duration < 300) {
            tapCount++;
        }
    }

    // Continuous Rub / Long Hold (> 0.7s) -> Trigger Shy Love Emoji 👉👈
    if (isTouching && (now - touchStartTime > 700)) {
        if (currentMode == MODE_CYBERPET) {
            memePet.setEmotion(EMOTION_SHY);
        } else if (currentMode == MODE_ROBOT_EYES) {
            robotEyes.setMood(MOOD_LOVE);
        }
        lastActivityTime = now;
    }

    // Multi-tap timeout window (250ms)
    if (!isTouching && tapCount > 0 && (now - touchReleaseTime > 250)) {
        if (tapCount >= 3) {
            // Rage Spam Tapped -> ANGRY GRUMPY KITTEN!
            if (currentMode == MODE_CYBERPET) {
                memePet.setEmotion(EMOTION_ANGRY_CAT);
            } else {
                robotEyes.setMood(MOOD_ANGRY);
            }
        } else if (tapCount == 2) {
            // Double Tap -> Banana Cat Sad / Bunny
            if (currentMode == MODE_CYBERPET) {
                memePet.setEmotion(EMOTION_SAD_BANANA);
            }
        } else if (tapCount == 1) {
            // Single Tap -> Cycle through Meme Avatars or Modes
            if (currentMode == MODE_CYBERPET) {
                memePet.setEmotion((MemeEmotion)((memePet.currentEmotion + 1) % 5));
            } else if (currentMode == MODE_ROBOT_EYES) {
                currentMode = MODE_CYBER_HUD;
            } else if (currentMode == MODE_CYBER_HUD) {
                currentMode = MODE_MATRIX_RAIN;
            } else if (currentMode == MODE_MATRIX_RAIN) {
                currentMode = MODE_TEXT_SCROLL;
            } else {
                currentMode = MODE_CYBERPET;
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
    for (int b = screenBrightness; b >= 0; b -= 20) {
        tft.setBrightness(b);
        delay(15);
    }
    tft.writeCommand(0x10); // ST7789 Sleep In command

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
    Serial.println("  ESP32-C3 MEME KEYCHAIN v3.0 READY");
    Serial.println("========================================");

    // Hardware Pins
    pinMode(PIN_DEBUG_LED, OUTPUT);
    pinMode(PIN_TOUCH, INPUT);

    // Display init (Rotated 270 deg / Landscape Left)
    tft.init();
    tft.setRotation(3);
    tft.setBrightness(screenBrightness);

    // Double Buffer Sprite
    canvas.setColorDepth(16);
    canvas.createSprite(240, 240);

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

    auto pCharStream = pService->createCharacteristic(CHAR_STREAM_UUID, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR);
    pCharStream->setCallbacks(new StreamCallback());

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

    // 2. Heartbeat LED
    static uint32_t lastBlink = 0;
    static bool ledState = false;
    if (millis() - lastBlink > 500) {
        lastBlink = millis();
        ledState = !ledState;
        digitalWrite(PIN_DEBUG_LED, ledState ? HIGH : LOW);
    }

    // 3. Render Active Mode to Double Buffer Sprite
    switch (currentMode) {
        case MODE_CYBERPET:
            memePet.update();
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
            // Sleek single border line around edges
            canvas.drawRoundRect(2, 2, 236, 236, 6, 0x07FF);

            // Screen-filling maxed out bold font centered vertically
            canvas.setTextColor(0x07E0, TFT_BLACK); // Bright Neon Green
            canvas.setTextSize(4); // Huge bold font
            canvas.drawString(customMessage, scrollX, 105);

            scrollX -= 5;
            if (scrollX < -((int)customMessage.length() * 26)) {
                scrollX = 240;
            }
            break;

        case MODE_STREAM_MEDIA:
            if (newMediaFrameReady && streamBytesReceived > 0) {
                canvas.fillScreen(TFT_BLACK);
                canvas.drawJpg(streamBuffer, streamBytesReceived, 0, 0, 240, 240);
                canvas.drawRoundRect(0, 0, 240, 240, 4, 0x07FF);
            }
            break;
    }

    // 4. Push Frame to Display (Hardware DMA Transfer)
    canvas.pushSprite(0, 0);

    // 5. Deep Sleep if Unplugged, Idle, and No BLE Connection
    if (!bleConnected && (millis() - lastActivityTime > sleepTimeoutMs)) {
        enterDeepSleep();
    }

    delay(10);
}


