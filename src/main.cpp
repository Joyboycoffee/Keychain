#include <Arduino.h>
#include <NimBLEDevice.h>
#include <Preferences.h>
#include "esp_sleep.h"
#include "driver/gpio.h"
#include "config.h"
#include "display_setup.h"
#include "meme_pet.h"
#include "robot_eyes.h"
#include "cyber_hud.h"
#include "matrix_rain.h"
#include "emoji_renderer.h"

// Instantiate Display & Double Buffer Sprite
LGFX_ST7789 tft;
LGFX_Sprite canvas(&tft);

// Non-Volatile Storage Preferences
Preferences prefs;

// Mode Engines
SystemMode currentMode = MODE_CYBERPET;
SystemMode defaultMode = MODE_CYBERPET;
MemePet memePet;
RobotEyes robotEyes;
CyberHUD cyberHUD;
MatrixRain matrixRain;

// Custom Scrolling Text with Emoji Support
String customMessage = "I AM JOY BOY COFFEE :coffee: :fire:";
int scrollX = 240;

// Power, Battery & Settings
uint8_t screenBrightness = 240; // 0-255 PWM
uint8_t screenRotation = 3;     // 0=0 deg, 1=90 deg, 2=180 deg, 3=270 deg
uint32_t sleepTimeoutMs = 30000; // 30s auto-sleep (0 = never)
uint32_t lastActivityTime = 0;
bool bleConnected = false;

// Battery Telemetry
float currentBatVoltage = 4.12f;
int currentBatPercent = 95;
bool isUsbPower = false;
uint32_t lastBatterySampleTime = 0;

// Touch Gesture Tracker
uint32_t touchStartTime = 0;
uint32_t touchReleaseTime = 0;
int tapCount = 0;
bool isTouching = false;

// BLE Characteristics (for bidirectional notifications)
NimBLECharacteristic* pCharMode = nullptr;
NimBLECharacteristic* pCharPet = nullptr;
NimBLECharacteristic* pCharBattery = nullptr;

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

// Forward declarations
void updateBatteryTelemetry();

// =========================================================================
// BLE CALLBACKS
// =========================================================================
class ServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer) {
        bleConnected = true;
        lastActivityTime = millis();
        Serial.println("[BLE] Phone Connected!");
    }
    void onDisconnect(NimBLEServer* pServer) {
        bleConnected = false;
        lastActivityTime = millis();
        Serial.println("[BLE] Phone Disconnected. Advertising restarted.");
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
                Serial.printf("[BLE] Switched Mode: %d\n", m);
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
                Serial.printf("[BLE] Switched Emotion: %d\n", a);
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
            prefs.putString("msg", customMessage);
            Serial.printf("[BLE] Marquee Text: %s\n", customMessage.c_str());
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
                cyberHUD.setWeather(temp, "SYNCED");
            }
            lastActivityTime = millis();
            Serial.printf("[BLE] Time Synced: %02d:%02d:%02d\n", h, m, s);
        }
    }
};

class SettingsCallback : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pChar) {
        std::string val = pChar->getValue();
        if (val.length() == 0) return;

        String cmd = String(val.c_str());
        lastActivityTime = millis();

        // 1. Rotation Command: "ROT:0", "ROT:1", "ROT:2", "ROT:3"
        if (cmd.startsWith("ROT:")) {
            int r = cmd.substring(4).toInt();
            if (r >= 0 && r <= 3) {
                screenRotation = r;
                tft.setRotation(screenRotation);
                prefs.putUChar("rot", screenRotation);
                Serial.printf("[SETTINGS] Rotation set: %d\n", r);
            }
        }
        // 2. Sleep Timeout Command: "SLEEP:30" (seconds)
        else if (cmd.startsWith("SLEEP:")) {
            int sec = cmd.substring(6).toInt();
            sleepTimeoutMs = (sec <= 0) ? 0 : (sec * 1000);
            prefs.putUInt("sleep", sleepTimeoutMs);
            Serial.printf("[SETTINGS] Sleep timer set: %u ms\n", (unsigned int)sleepTimeoutMs);
        }
        // 3. Set Current As Default: "DEF:SET"
        else if (cmd.startsWith("DEF:SET")) {
            defaultMode = currentMode;
            memePet.defaultEmotion = memePet.currentEmotion;
            prefs.putUChar("def_mode", (uint8_t)defaultMode);
            prefs.putUChar("def_emo", (uint8_t)memePet.defaultEmotion);
            Serial.printf("[SETTINGS] Saved default mode: %d, emotion: %d\n", (int)defaultMode, (int)memePet.defaultEmotion);
        }
        // 4. Brightness Number
        else {
            int br = cmd.toInt();
            if (br >= 10 && br <= 255) {
                screenBrightness = br;
                tft.setBrightness(screenBrightness);
                prefs.putUChar("br", screenBrightness);
                Serial.printf("[SETTINGS] Brightness: %d\n", br);
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

        // 1. Single Image Header: 0xAA 0x55 [len_hi] [len_lo]
        if (len >= 4 && bytes[0] == 0xAA && bytes[1] == 0x55) {
            expectedStreamBytes = (bytes[2] << 8) | bytes[3];
            streamBytesReceived = 0;
            isVideoPlaying = false;
            totalVideoFrames = 0;
            newMediaFrameReady = false;
            if (len > 4) {
                size_t payload = len - 4;
                if (payload <= STREAM_BUFFER_SIZE) {
                    memcpy(streamBuffer, bytes + 4, payload);
                    streamBytesReceived = payload;
                }
            }
            return;
        }

        // 2. Video Init Header: 0xBB 0x66 [total_frames] [fps]
        if (len >= 4 && bytes[0] == 0xBB && bytes[1] == 0x66) {
            totalVideoFrames = bytes[2];
            if (totalVideoFrames > MAX_VIDEO_FRAMES) totalVideoFrames = MAX_VIDEO_FRAMES;
            videoTargetFps = bytes[3] > 0 ? bytes[3] : 30;
            videoPoolWriteOffset = 0;
            currentVideoFrame = 0;
            isVideoPlaying = false;
            incomingFrameIdx = -1;
            Serial.printf("[STREAM] Video Init: %d frames @ %d FPS\n", totalVideoFrames, videoTargetFps);
            return;
        }

        // 3. Video Frame Header: 0xCC 0x77 [frame_idx] [len_hi] [len_lo]
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

        // 4. Video Play Header: 0xDD 0x88
        if (len >= 2 && bytes[0] == 0xDD && bytes[1] == 0x88) {
            isVideoPlaying = true;
            currentVideoFrame = 0;
            currentMode = MODE_STREAM_MEDIA;
            lastVideoFrameTime = millis();
            lastActivityTime = millis();
            Serial.printf("[STREAM] Playback started (%d frames @ %d FPS)!\n", totalVideoFrames, videoTargetFps);
            return;
        }

        // 5. Append Video Frame Chunk
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

        // 6. Append Single Image Chunk
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
                Serial.printf("[STREAM] Image ready (%d bytes)!\n", (int)streamBytesReceived);
            }
        }
    }
};

// =========================================================================
// BATTERY VOLTAGE SENSING (100k + 100k DIVIDER ON GPIO 0)
// =========================================================================
void updateBatteryTelemetry() {
    uint32_t rawMv = analogReadMilliVolts(PIN_BAT_ADC);
    float vbat = (rawMv * 2.0f) / 1000.0f;
    
    if (vbat < 2.5f) {
        currentBatVoltage = 5.0f;
        currentBatPercent = 100;
        isUsbPower = true;
    } else {
        currentBatVoltage = vbat;
        isUsbPower = false;
        if (vbat >= 4.15f) currentBatPercent = 100;
        else if (vbat <= 3.20f) currentBatPercent = 0;
        else currentBatPercent = (int)((vbat - 3.20f) / (4.15f - 3.20f) * 100.0f);
    }

    cyberHUD.setBattery(currentBatVoltage, currentBatPercent, isUsbPower);

    if (bleConnected && pCharBattery) {
        char bMsg[24];
        snprintf(bMsg, sizeof(bMsg), "%.2f|%d|%d", currentBatVoltage, currentBatPercent, isUsbPower ? 1 : 0);
        pCharBattery->setValue(bMsg);
        pCharBattery->notify();
    }
}

// =========================================================================
// TOUCH GESTURES (INTENTIONAL DOUBLE-TAP & HOLD, WITH LIVE BLE UI NOTIFICATION)
// =========================================================================
void processTouch() {
    bool rawTouch = digitalRead(PIN_TOUCH) == HIGH;
    uint32_t now = millis();

    if (rawTouch && !isTouching) {
        isTouching = true;
        touchStartTime = now;
        lastActivityTime = now;
    } else if (!rawTouch && isTouching) {
        isTouching = false;
        touchReleaseTime = now;
        uint32_t duration = touchReleaseTime - touchStartTime;

        if (duration < 350) {
            tapCount++;
        }
    }

    // 1. Long Hold (> 0.5s) -> Trigger Shy Love Emoji 👉👈
    if (isTouching && (now - touchStartTime > 500)) {
        if (currentMode == MODE_CYBERPET && memePet.currentEmotion != EMOTION_SHY) {
            memePet.setEmotion(EMOTION_SHY);
            if (bleConnected && pCharPet) {
                pCharPet->setValue(String((int)EMOTION_SHY));
                pCharPet->notify();
            }
            Serial.println("[TOUCH] Hold -> Shy Love 👉👈");
        } else if (currentMode == MODE_ROBOT_EYES) {
            robotEyes.setMood(MOOD_LOVE);
        }
        tapCount = 0;
        lastActivityTime = now;
    }

    // 2. Multi-Tap Execution Window (280ms)
    if (!isTouching && tapCount > 0 && (now - touchReleaseTime > 280)) {
        if (tapCount >= 3) {
            // Triple Tap -> Grumpy Cat
            if (currentMode == MODE_CYBERPET) {
                memePet.setEmotion(EMOTION_ANGRY_CAT);
                if (bleConnected && pCharPet) {
                    pCharPet->setValue(String((int)EMOTION_ANGRY_CAT));
                    pCharPet->notify();
                }
                Serial.println("[TOUCH] 3x -> Grumpy Cat 😾");
            }
        } else if (tapCount == 2) {
            // Double Tap -> Cycle through interactive meme emotions
            if (currentMode == MODE_CYBERPET) {
                MemeEmotion nextEmo = (MemeEmotion)((memePet.currentEmotion + 1) % 7);
                if (nextEmo == memePet.defaultEmotion) nextEmo = (MemeEmotion)((nextEmo + 1) % 7);
                memePet.setEmotion(nextEmo);
                if (bleConnected && pCharPet) {
                    pCharPet->setValue(String((int)nextEmo));
                    pCharPet->notify();
                }
                Serial.printf("[TOUCH] 2x -> Emotion: %d\n", (int)nextEmo);
            } else if (currentMode == MODE_ROBOT_EYES) {
                currentMode = MODE_CYBER_HUD;
                if (bleConnected && pCharMode) { pCharMode->setValue(String((int)currentMode)); pCharMode->notify(); }
            } else if (currentMode == MODE_CYBER_HUD) {
                currentMode = MODE_MATRIX_RAIN;
                if (bleConnected && pCharMode) { pCharMode->setValue(String((int)currentMode)); pCharMode->notify(); }
            } else if (currentMode == MODE_MATRIX_RAIN) {
                currentMode = MODE_TEXT_SCROLL;
                if (bleConnected && pCharMode) { pCharMode->setValue(String((int)currentMode)); pCharMode->notify(); }
            } else {
                currentMode = MODE_CYBERPET;
                if (bleConnected && pCharMode) { pCharMode->setValue(String((int)currentMode)); pCharMode->notify(); }
            }
        } else if (tapCount == 1) {
            Serial.println("[TOUCH] 1x -> Interaction poke");
        }
        tapCount = 0;
        lastActivityTime = now;
    }
}

// =========================================================================
// DEEP SLEEP ROUTINE (100% PITCH BLACK BACKLIGHT SHUTOFF)
// =========================================================================
void enterDeepSleep() {
    Serial.println("[POWER] Entering Deep Sleep (100% Backlight OFF)...");
    
    for (int b = screenBrightness; b >= 0; b -= 30) {
        tft.setBrightness(b);
        delay(10);
    }
    tft.setBrightness(0);
    
    tft.writeCommand(0x28); // Display OFF
    tft.writeCommand(0x10); // Sleep IN
    delay(20);

    pinMode(PIN_TFT_BL, OUTPUT);
    digitalWrite(PIN_TFT_BL, LOW);
    gpio_set_direction((gpio_num_t)PIN_TFT_BL, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)PIN_TFT_BL, 0);
    gpio_hold_en((gpio_num_t)PIN_TFT_BL);
    gpio_deep_sleep_hold_en();

    gpio_wakeup_enable((gpio_num_t)PIN_TOUCH, GPIO_INTR_HIGH_LEVEL);
    esp_deep_sleep_enable_gpio_wakeup(1ULL << PIN_TOUCH, ESP_GPIO_WAKEUP_GPIO_HIGH);

    esp_deep_sleep_start();
}

// =========================================================================
// 2-SECOND JOYBOYCOFFEE CYBER BOOT INTRO ANIMATION
// =========================================================================
void playBootAnimation() {
    Serial.println("[BOOT] Playing 2-second Joyboycoffee Cyber Intro...");
    uint32_t start = millis();

    while (millis() - start < 2000) {
        uint32_t elapsed = millis() - start;
        float progress = (float)elapsed / 2000.0f;

        canvas.fillScreen(TFT_BLACK);

        int ringR = (int)(progress * 110.0f);
        canvas.drawCircle(120, 100, ringR % 90, 0x07FF);
        canvas.drawCircle(120, 100, (ringR + 30) % 90, 0x0210);

        int cx = 120 - 10;
        int cy = 80;
        canvas.fillRoundRect(cx, cy, 20, 16, 4, 0xD440);
        canvas.fillRect(cx + 2, cy + 2, 16, 3, 0x5180);
        canvas.drawRoundRect(cx + 17, cy + 3, 7, 10, 2, 0xD440);

        int steamOffset = (elapsed / 40) % 10;
        canvas.drawFastVLine(cx + 5, cy - 3 - steamOffset, 5, 0xFFFF);
        canvas.drawFastVLine(cx + 10, cy - 5 - ((steamOffset + 4) % 10), 6, 0xFFFF);
        canvas.drawFastVLine(cx + 15, cy - 2 - ((steamOffset + 7) % 10), 4, 0xFFFF);

        canvas.setTextColor(0xFFE0, TFT_BLACK);
        canvas.setTextSize(2);
        canvas.drawCenterString("JOYBOYCOFFEE", 120, 125);

        canvas.setTextColor(0x07FF, TFT_BLACK);
        canvas.setTextSize(1);
        canvas.drawCenterString("DIGI_KEYCHAIN // V4.0", 120, 155);

        int barW = (int)(progress * 180.0f);
        canvas.drawRoundRect(30, 185, 180, 8, 3, 0x07FF);
        canvas.fillRect(32, 187, barW, 4, 0x07E0);

        canvas.pushSprite(0, 0);
        delay(20);
    }
}

// =========================================================================
// MARQUEE TEXT WITH EMOJI RENDERING ENGINE
// =========================================================================
void renderMarqueeText() {
    canvas.fillScreen(TFT_BLACK);
    canvas.drawRoundRect(2, 2, 236, 236, 6, 0x07FF);

    int curX = scrollX;
    int curY = 108;
    int len = customMessage.length();
    int i = 0;

    while (i < len) {
        EmojiType em = EMOJI_NONE;
        int skipLen = 0;

        if (customMessage.substring(i).startsWith(":coffee:"))   { em = EMOJI_COFFEE;   skipLen = 8; }
        else if (customMessage.substring(i).startsWith(":heart:"))  { em = EMOJI_HEART;    skipLen = 7; }
        else if (customMessage.substring(i).startsWith(":fire:"))   { em = EMOJI_FIRE;     skipLen = 6; }
        else if (customMessage.substring(i).startsWith(":star:"))   { em = EMOJI_STAR;     skipLen = 6; }
        else if (customMessage.substring(i).startsWith(":sparkles:")){ em = EMOJI_SPARKLES; skipLen = 10; }
        else if (customMessage.substring(i).startsWith(":cat:"))    { em = EMOJI_CAT;      skipLen = 5; }
        else if (customMessage.substring(i).startsWith(":cry:"))    { em = EMOJI_CRY;      skipLen = 5; }
        else if (customMessage.substring(i).startsWith(":banana:")) { em = EMOJI_BANANA;   skipLen = 8; }
        else if (customMessage.substring(i).startsWith(":skull:"))  { em = EMOJI_SKULL;    skipLen = 7; }
        else if (customMessage.substring(i).startsWith(":rocket:")) { em = EMOJI_ROCKET;   skipLen = 8; }
        else if ((uint8_t)customMessage[i] == 0xF0 && (uint8_t)customMessage[i+1] == 0x9F) {
            uint8_t b2 = (uint8_t)customMessage[i+2];
            uint8_t b3 = (uint8_t)customMessage[i+3];
            skipLen = 4;
            if (b2 == 0x8D && b3 == 0xB5) em = EMOJI_COFFEE;
            else if (b2 == 0x94 && b3 == 0xA5) em = EMOJI_FIRE;
            else if (b2 == 0x90 && b3 == 0xB1) em = EMOJI_CAT;
            else if (b2 == 0x98 && b3 == 0xAD) em = EMOJI_CRY;
            else if (b2 == 0x8D && b3 == 0x8C) em = EMOJI_BANANA;
            else if (b2 == 0x92 && b3 == 0x80) em = EMOJI_SKULL;
            else if (b2 == 0x9A && b3 == 0x80) em = EMOJI_ROCKET;
        } else if ((uint8_t)customMessage[i] == 0xE2) {
            uint8_t b2 = (uint8_t)customMessage[i+1];
            skipLen = 3;
            if (b2 == 0x9D) em = EMOJI_HEART;
            else if (b2 == 0xAD) em = EMOJI_STAR;
            else if (b2 == 0x9C) em = EMOJI_SPARKLES;
        }

        if (em != EMOJI_NONE) {
            if (curX > -24 && curX < 240) {
                EmojiRenderer::drawEmoji(&canvas, em, curX, curY - 6);
            }
            curX += 28;
            i += skipLen;
        } else {
            char c = customMessage[i];
            if (curX > -20 && curX < 240) {
                canvas.setTextColor(0x07E0, TFT_BLACK);
                canvas.setTextSize(3);
                canvas.drawChar(c, curX, curY);
            }
            curX += 20;
            i++;
        }
    }

    scrollX -= 4;
    int totalPixelWidth = curX - scrollX;
    if (scrollX < -totalPixelWidth) {
        scrollX = 240;
    }
}

// =========================================================================
// SETUP
// =========================================================================
void setup() {
    Serial.begin(115200);
    delay(250);
    Serial.println("\n\n========================================");
    Serial.println("  ESP32-C3 DIGI KEYCHAIN v4.0 BOOT");
    Serial.println("========================================");

    gpio_hold_dis((gpio_num_t)PIN_TFT_BL);
    gpio_deep_sleep_hold_dis();

    pinMode(PIN_DEBUG_LED, OUTPUT);
    digitalWrite(PIN_DEBUG_LED, HIGH);
    pinMode(PIN_TOUCH, INPUT);
    analogSetPinAttenuation(PIN_BAT_ADC, ADC_11db);
    pinMode(PIN_BAT_ADC, INPUT);

    prefs.begin("digi_keychain", false);
    defaultMode = (SystemMode)prefs.getUChar("def_mode", (uint8_t)MODE_CYBERPET);
    currentMode = defaultMode;
    memePet.defaultEmotion = (MemeEmotion)prefs.getUChar("def_emo", (uint8_t)EMOTION_LUFFY);
    memePet.currentEmotion = memePet.defaultEmotion;
    screenBrightness = prefs.getUChar("br", 240);
    screenRotation = prefs.getUChar("rot", 3);
    sleepTimeoutMs = prefs.getUInt("sleep", 30000);
    customMessage = prefs.getString("msg", "I AM JOY BOY COFFEE :coffee: :fire:");

    Serial.println("[BLE] Initializing NimBLE stack...");
    NimBLEDevice::init("DIGI_KEYCHAIN");
    NimBLEServer* pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());

    NimBLEService* pService = pServer->createService(SERVICE_UUID);

    pCharMode = pService->createCharacteristic(CHAR_MODE_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY);
    pCharMode->setCallbacks(new ModeCallback());

    pCharPet = pService->createCharacteristic(CHAR_PET_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY);
    pCharPet->setCallbacks(new PetCallback());

    auto pCharText = pService->createCharacteristic(CHAR_TEXT_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
    pCharText->setCallbacks(new TextCallback());

    auto pCharTime = pService->createCharacteristic(CHAR_TIME_UUID, NIMBLE_PROPERTY::WRITE);
    pCharTime->setCallbacks(new TimeCallback());

    auto pCharSet = pService->createCharacteristic(CHAR_SETTINGS_UUID, NIMBLE_PROPERTY::WRITE);
    pCharSet->setCallbacks(new SettingsCallback());

    pCharBattery = pService->createCharacteristic(CHAR_BATTERY_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);

    auto pCharStream = pService->createCharacteristic(CHAR_STREAM_UUID, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR);
    pCharStream->setCallbacks(new StreamCallback());

    pService->start();

    NimBLEAdvertising* pAdv = NimBLEDevice::getAdvertising();
    pAdv->addServiceUUID(SERVICE_UUID);
    pAdv->start();
    Serial.println("[BLE] Advertising started as DIGI_KEYCHAIN (0xFFE0)");

    Serial.println("[DISPLAY] Initializing ST7789 display...");
    tft.init();
    tft.setRotation(screenRotation);
    tft.setBrightness(screenBrightness);

    canvas.setColorDepth(16);
    canvas.createSprite(240, 240);

    esp_sleep_wakeup_cause_t wakeupReason = esp_sleep_get_wakeup_cause();
    if (wakeupReason == ESP_SLEEP_WAKEUP_UNDEFINED) {
        playBootAnimation();
    } else {
        Serial.printf("[BOOT] Woke from Deep Sleep (%d). Skipping intro.\n", (int)wakeupReason);
    }

    updateBatteryTelemetry();
    lastActivityTime = millis();
    Serial.println("[SYSTEM] Ready! Running active engine.");
}

// =========================================================================
// MAIN LOOP
// =========================================================================
void loop() {
    processTouch();

    if (millis() - lastBatterySampleTime > 2500) {
        lastBatterySampleTime = millis();
        updateBatteryTelemetry();
    }

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
            renderMarqueeText();
            break;

        case MODE_STREAM_MEDIA:
            if (isVideoPlaying && totalVideoFrames > 0) {
                uint32_t frameInterval = 1000 / videoTargetFps;
                if (millis() - lastVideoFrameTime >= frameInterval) {
                    lastVideoFrameTime = millis();
                    if (frameLengths[currentVideoFrame] > 0) {
                        canvas.fillScreen(TFT_BLACK);
                        canvas.drawJpg(videoPool + frameOffsets[currentVideoFrame], frameLengths[currentVideoFrame], 0, 0);
                        canvas.drawRoundRect(0, 0, 240, 240, 4, 0x07FF);
                    }
                    currentVideoFrame = (currentVideoFrame + 1) % totalVideoFrames;
                }
            } else if (newMediaFrameReady && streamBytesReceived > 0) {
                canvas.fillScreen(TFT_BLACK);
                canvas.drawJpg(streamBuffer, streamBytesReceived, 0, 0);
                canvas.drawRoundRect(0, 0, 240, 240, 4, 0x07FF);
            }
            break;
    }

    canvas.pushSprite(0, 0);

    if (!bleConnected && sleepTimeoutMs > 0 && (millis() - lastActivityTime > sleepTimeoutMs)) {
        enterDeepSleep();
    }

    delay(10);
}
