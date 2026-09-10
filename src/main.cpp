#include <Arduino.h>
#include <NimBLEDevice.h>
#include "esp_sleep.h"
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
uint8_t screenBrightness = 240; // 0-255 PWM
uint32_t sleepTimeoutMs = 30000; // 30 seconds auto-sleep on inactivity
uint32_t lastActivityTime = 0;
bool bleConnected = false;

// Touch Gesture Tracker
uint32_t touchStartTime = 0;
uint32_t touchReleaseTime = 0;
int tapCount = 0;
bool isTouching = false;

// Media & 30 FPS Video Buffers
#define STREAM_BUFFER_SIZE 32768
uint8_t streamBuffer[STREAM_BUFFER_SIZE];
size_t streamBytesReceived = 0;
size_t expectedStreamBytes = 0;
bool newMediaFrameReady = false;

#define MAX_VIDEO_FRAMES 30
#define VIDEO_POOL_SIZE 32768
uint8_t videoPool[VIDEO_POOL_SIZE];
size_t videoPoolWriteOffset = 0;
uint32_t frameOffsets[MAX_VIDEO_FRAMES];
uint16_t frameLengths[MAX_VIDEO_FRAMES];
int totalVideoFrames = 0;
int currentVideoFrame = 0;
int videoTargetFps = 30;
uint32_t lastVideoFrameTime = 0;
bool isVideoPlaying = false;

int incomingFrameIdx = -1;
size_t incomingFrameExpected = 0;
size_t incomingFrameReceived = 0;

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
                if (currentMode != MODE_STREAM_MEDIA) isVideoPlaying = false;
                lastActivityTime = millis();
                Serial.printf("[BLE] Mode: %d\n", m);
            }
        }
    }
};

class PetCallback : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pChar) {
        std::string val = pChar->getValue();
        if (val.length() > 0) {
            int a = val[0] - '0';
            if (a >= 0 && a <= 6) {
                memePet.setEmotion((MemeEmotion)a);
                currentMode = MODE_CYBERPET;
                isVideoPlaying = false;
                lastActivityTime = millis();
                Serial.printf("[BLE] Emotion: %d\n", a);
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
            isVideoPlaying = false;
            lastActivityTime = millis();
            Serial.printf("[BLE] Text: %s\n", customMessage.c_str());
        }
    }
};

class TimeCallback : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pChar) {
        std::string val = pChar->getValue();
        if (val.length() >= 8) {
            int h = atoi(val.substr(0, 2).c_str());
            int m = atoi(val.substr(3, 2).c_str());
            int s = atoi(val.substr(6, 2).c_str());
            cyberHUD.setTime(h, m, s);
            
            size_t barIdx = val.find('|');
            if (barIdx != std::string::npos) {
                float temp = atof(val.substr(barIdx + 1).c_str());
                cyberHUD.setWeather(temp, "SYNC");
            }
            lastActivityTime = millis();
            Serial.printf("[BLE] Time Synced: %02d:%02d:%02d\n", h, m, s);
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
                Serial.printf("[BLE] Brightness: %d\n", br);
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

        if (len >= 4 && bytes[0] == 0xAA && bytes[1] == 0x55) {
            expectedStreamBytes = (bytes[2] << 8) | bytes[3];
            streamBytesReceived = 0;
            isVideoPlaying = false;
            totalVideoFrames = 0;
            if (len > 4) {
                size_t payload = len - 4;
                if (payload <= STREAM_BUFFER_SIZE) {
                    memcpy(streamBuffer, bytes + 4, payload);
                    streamBytesReceived = payload;
                }
            }
            return;
        }

        if (len >= 4 && bytes[0] == 0xBB && bytes[1] == 0x66) {
            totalVideoFrames = bytes[2];
            if (totalVideoFrames > MAX_VIDEO_FRAMES) totalVideoFrames = MAX_VIDEO_FRAMES;
            videoTargetFps = bytes[3] > 0 ? bytes[3] : 30;
            videoPoolWriteOffset = 0;
            currentVideoFrame = 0;
            isVideoPlaying = false;
            incomingFrameIdx = -1;
            Serial.printf("[BLE-VIDEO] Init %d frames @ %d FPS\n", totalVideoFrames, videoTargetFps);
            return;
        }

        if (len >= 5 && bytes[0] == 0xCC && bytes[1] == 0x77) {
            incomingFrameIdx = bytes[2];
            incomingFrameExpected = (bytes[3] << 8) | bytes[4];
            incomingFrameReceived = 0;

            if (incomingFrameIdx < MAX_VIDEO_FRAMES && (videoPoolWriteOffset + incomingFrameExpected) <= VIDEO_POOL_SIZE) {
                frameOffsets[incomingFrameIdx] = videoPoolWriteOffset;
                frameLengths[incomingFrameIdx] = 0;
                size_t payload = len - 5;
                if (payload > 0) {
                    memcpy(videoPool + videoPoolWriteOffset, bytes + 5, payload);
                    videoPoolWriteOffset += payload;
                    incomingFrameReceived += payload;
                }
            }
            return;
        }

        if (len >= 2 && bytes[0] == 0xDD && bytes[1] == 0x88) {
            isVideoPlaying = true;
            currentVideoFrame = 0;
            currentMode = MODE_STREAM_MEDIA;
            lastVideoFrameTime = millis();
            lastActivityTime = millis();
            Serial.printf("[BLE-VIDEO] Playback STARTED @ %d FPS!\n", videoTargetFps);
            return;
        }

        if (incomingFrameIdx >= 0 && incomingFrameReceived < incomingFrameExpected) {
            if (videoPoolWriteOffset + len <= VIDEO_POOL_SIZE) {
                memcpy(videoPool + videoPoolWriteOffset, bytes, len);
                videoPoolWriteOffset += len;
                incomingFrameReceived += len;
                if (incomingFrameReceived >= incomingFrameExpected) {
                    frameLengths[incomingFrameIdx] = incomingFrameExpected;
                    incomingFrameIdx = -1;
                }
            }
            return;
        }

        if (expectedStreamBytes > 0 && streamBytesReceived < expectedStreamBytes) {
            if (streamBytesReceived + len <= STREAM_BUFFER_SIZE) {
                memcpy(streamBuffer + streamBytesReceived, bytes, len);
                streamBytesReceived += len;
            }
            if (streamBytesReceived >= expectedStreamBytes) {
                newMediaFrameReady = true;
                isVideoPlaying = false;
                currentMode = MODE_STREAM_MEDIA;
                lastActivityTime = millis();
            }
        }
    }
};

// =========================================================================
// TOUCH GESTURES (RESPONSIVE & IMMEDIATE)
// =========================================================================
void processTouch() {
    bool rawTouch = digitalRead(PIN_TOUCH) == HIGH;
    uint32_t now = millis();

    if (rawTouch && !isTouching) {
        isTouching = true;
        touchStartTime = now;
        lastActivityTime = now;
        Serial.println("[TOUCH] Touch Pressed!");
    } else if (!rawTouch && isTouching) {
        isTouching = false;
        touchReleaseTime = now;
        uint32_t duration = touchReleaseTime - touchStartTime;
        Serial.printf("[TOUCH] Released (duration=%ums)\n", (unsigned int)duration);

        if (duration < 450) {
            tapCount++;
        }
    }

    // Long Hold (> 0.55s) -> Trigger Shy Love Emoji 👉👈
    if (isTouching && (now - touchStartTime > 550)) {
        if (currentMode == MODE_CYBERPET && memePet.currentEmotion != EMOTION_SHY) {
            memePet.setEmotion(EMOTION_SHY);
            Serial.println("[TOUCH] Hold -> Shy Love");
        } else if (currentMode == MODE_ROBOT_EYES) {
            robotEyes.setMood(MOOD_LOVE);
        }
        lastActivityTime = now;
    }

    // Process Taps (180ms window)
    if (!isTouching && tapCount > 0 && (now - touchReleaseTime > 180)) {
        if (tapCount >= 3) {
            if (currentMode == MODE_CYBERPET) memePet.setEmotion(EMOTION_ANGRY_CAT);
            else robotEyes.setMood(MOOD_ANGRY);
            Serial.println("[TOUCH] 3x -> Grumpy Cat");
        } else if (tapCount == 2) {
            if (currentMode == MODE_CYBERPET) memePet.setEmotion(EMOTION_SAD_BANANA);
            Serial.println("[TOUCH] 2x -> Sad Banana");
        } else if (tapCount == 1) {
            if (currentMode == MODE_CYBERPET) {
                memePet.setEmotion((MemeEmotion)((memePet.currentEmotion + 1) % 7));
                Serial.printf("[TOUCH] 1x -> Switched to Avatar: %d\n", memePet.currentEmotion);
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
// DEEP SLEEP ROUTINE (30 SECONDS INACTIVITY)
// =========================================================================
void enterDeepSleep() {
    Serial.println("[POWER] 30s inactivity reached. Entering Deep Sleep. Wakeup on Touch GPIO 1 or RST...");
    for (int b = screenBrightness; b >= 0; b -= 20) {
        tft.setBrightness(b);
        delay(15);
    }
    tft.setBrightness(0);
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
    delay(300);
    Serial.println("\n\n========================================");
    Serial.println("  ESP32-C3 MEME KEYCHAIN v3.5 BOOT");
    Serial.println("========================================");

    // 1. Hardware Pins
    pinMode(PIN_DEBUG_LED, OUTPUT);
    digitalWrite(PIN_DEBUG_LED, HIGH); // Permanently OFF
    pinMode(PIN_TOUCH, INPUT);

    // 2. Initialize NimBLE Bluetooth FIRST (guarantees contiguous BT memory)
    Serial.println("[BLE] Initializing NimBLE stack...");
    NimBLEDevice::init("CYBER_KEYCHAIN");
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
    Serial.println("[BLE] Advertising started as CYBER_KEYCHAIN (0xFFE0)");

    // 3. Initialize Display
    Serial.println("[DISPLAY] Initializing ST7789 display...");
    tft.init();
    tft.setRotation(3);
    tft.setBrightness(screenBrightness);

    // 4. Create Double Buffer Canvas Sprite
    Serial.println("[DISPLAY] Creating canvas double buffer sprite...");
    canvas.setColorDepth(16);
    void* ptr = canvas.createSprite(240, 240);
    Serial.printf("[DISPLAY] Canvas ptr: %p, free heap: %u bytes\n", ptr, (unsigned int)ESP.getFreeHeap());

    // 5. Cold Boot Splash vs Deep Sleep Wakeup
    esp_sleep_wakeup_cause_t wakeupReason = esp_sleep_get_wakeup_cause();
    if (wakeupReason == ESP_SLEEP_WAKEUP_UNDEFINED) {
        Serial.println("[BOOT] Cold Power-On. Showing KEYCHAIN POWER ON...");
        canvas.fillScreen(TFT_BLACK);
        canvas.drawRoundRect(2, 2, 236, 236, 6, 0x07FF);
        canvas.drawRoundRect(3, 3, 234, 234, 5, 0x07FF);
        
        canvas.setTextColor(0x07FF, TFT_BLACK);
        canvas.setTextSize(2);
        canvas.drawCenterString("KEYCHAIN", 120, 85);
        
        canvas.setTextColor(0x07E0, TFT_BLACK);
        canvas.setTextSize(2);
        canvas.drawCenterString("POWER ON", 120, 125);
        
        canvas.pushSprite(0, 0);
        delay(600);
    } else {
        Serial.printf("[BOOT] Woke from Deep Sleep (%d). Skipping splash.\n", (int)wakeupReason);
    }

    lastActivityTime = millis();
    Serial.println("[SYSTEM] Setup completed! Entering main loop.");
}

// =========================================================================
// MAIN LOOP
// =========================================================================
void loop() {
    // 1. Process Touch Gestures
    processTouch();

    // 2. Render Active Mode to Double Buffer Sprite
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
            canvas.drawRoundRect(2, 2, 236, 236, 6, 0x07FF);
            canvas.setTextColor(0x07E0, TFT_BLACK);
            canvas.setTextSize(4);
            canvas.drawString(customMessage, scrollX, 105);

            scrollX -= 5;
            if (scrollX < -((int)customMessage.length() * 26)) {
                scrollX = 240;
            }
            break;

        case MODE_STREAM_MEDIA:
            if (isVideoPlaying && totalVideoFrames > 0) {
                uint32_t frameInterval = 1000 / videoTargetFps;
                if (millis() - lastVideoFrameTime >= frameInterval) {
                    lastVideoFrameTime = millis();
                    if (frameLengths[currentVideoFrame] > 0) {
                        canvas.drawJpg(videoPool + frameOffsets[currentVideoFrame], frameLengths[currentVideoFrame], 0, 0, 240, 240);
                    }
                    currentVideoFrame = (currentVideoFrame + 1) % totalVideoFrames;
                }
            } else if (newMediaFrameReady && streamBytesReceived > 0) {
                canvas.fillScreen(TFT_BLACK);
                canvas.drawJpg(streamBuffer, streamBytesReceived, 0, 0, 240, 240);
                canvas.drawRoundRect(0, 0, 240, 240, 4, 0x07FF);
            }
            break;
    }

    // 3. Push Frame to Display (Hardware DMA Transfer)
    canvas.pushSprite(0, 0);

    // 4. Deep Sleep if Unplugged, Idle, and No BLE Connection (30s)
    if (!bleConnected && (millis() - lastActivityTime > sleepTimeoutMs)) {
        enterDeepSleep();
    }

    delay(10);
}
