#include <Arduino.h>
#include <LovyanGFX.hpp>
#include <NimBLEDevice.h>
#include <Preferences.h>
#include <LittleFS.h>
#include "driver/gpio.h"
#include "esp_sleep.h"
#include "config.h"
#include "display_setup.h"
#include "meme_pet.h"
#include "robot_eyes.h"
#include "cyber_hud.h"
#include "matrix_rain.h"
#include "emoji_renderer.h"

// Global Display Objects
LGFX_ST7789 tft;
LGFX_Sprite canvas(&tft);

// =========================================================================
// GLOBAL ENGINE OBJECTS & STATE
// =========================================================================
Preferences prefs;

MemePet     memePet;
RobotEyes   robotEyes;
CyberHUD    cyberHUD;
MatrixRain  matrixRain;

SystemMode     currentMode      = MODE_CYBERPET;
SystemMode     defaultMode      = MODE_CYBERPET;
BootSplashType bootSplashType   = BOOT_JOYBOY_INTRO;
uint8_t        bootDurationSec  = 2;
uint8_t        screenBrightness = 240;
uint8_t        screenRotation   = 3;
uint32_t       sleepTimeoutMs   = 60000;

bool bleConnected = false;

// Scrolling Marquee Message State
String customMessage = "I AM JOY BOY COFFEE :coffee: :fire:";
int scrollX = 240;

// Battery Telemetry
float currentBatVoltage = 4.20f;
int   currentBatPercent = 100;
bool  isUsbPower        = true;

// Touch State & Gestures
bool     isTouching          = false;
uint32_t touchStartTime      = 0;
uint32_t lastTapReleaseTime  = 0;
int      tapCount            = 0;
uint32_t lastActivityTime    = 0;

// On-Screen Touch Visualizer
bool     touchVisualActive   = false;
String   touchVisualText     = "TOUCH";
uint16_t touchVisualColor    = 0x07FF;
uint32_t touchVisualEndTime  = 0;

// Media Streaming Buffers
#define STREAM_BUFFER_SIZE 28000
uint8_t streamBuffer[STREAM_BUFFER_SIZE];
size_t  streamBytesReceived = 0;
size_t  expectedStreamBytes = 0;
bool    newMediaFrameReady  = false;

// 30 FPS Video Chunk Pool
#define VIDEO_POOL_SIZE 180000
#define MAX_VIDEO_FRAMES 40
uint8_t  videoPool[VIDEO_POOL_SIZE];
size_t   videoPoolWriteOffset = 0;
size_t   frameOffsets[MAX_VIDEO_FRAMES];
size_t   frameLengths[MAX_VIDEO_FRAMES];
int      totalVideoFrames     = 0;
int      currentVideoFrame    = 0;
uint8_t  videoTargetFps       = 30;
bool     isVideoPlaying       = false;
uint32_t lastVideoFrameTime   = 0;
int      incomingFrameIdx     = -1;
size_t   incomingFrameExpected= 0;
size_t   incomingFrameReceived= 0;

// Boot Splash Upload State
File     bootFile;
bool     isBootUploading      = false;
uint8_t  bootUploadType       = 0; // 1 = image, 2 = video
size_t   bootBytesExpected    = 0;
size_t   bootBytesReceived    = 0;

// BLE Characteristic Pointers for Notifications
NimBLECharacteristic* pCharMode    = nullptr;
NimBLECharacteristic* pCharPet     = nullptr;
NimBLECharacteristic* pCharBattery = nullptr;
NimBLECharacteristic* pCharSet     = nullptr;

// Forward declarations
void updateBatteryTelemetry();
void playBootSplash();
void triggerTouchVisual(const String& label, uint16_t color, uint32_t durationMs, const char* bleState);

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
        Serial.println("[BLE] Client Disconnected. Advertising restarted.");
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
            Serial.printf("[SETTINGS] Default Boot screen saved: Mode=%d, Emo=%d\n", (int)defaultMode, (int)memePet.defaultEmotion);
        }
        // 4. Boot Splash Type: "BOOT_TYPE:0" .. "BOOT_TYPE:3"
        else if (cmd.startsWith("BOOT_TYPE:")) {
            int bt = cmd.substring(10).toInt();
            if (bt >= 0 && bt <= 3) {
                bootSplashType = (BootSplashType)bt;
                prefs.putUChar("boot_type", (uint8_t)bootSplashType);
                Serial.printf("[SETTINGS] Boot Splash Type: %d\n", bt);
            }
        }
        // 5. Boot Splash Duration: "BOOT_DUR:2" (seconds)
        else if (cmd.startsWith("BOOT_DUR:")) {
            int dur = cmd.substring(9).toInt();
            if (dur >= 1 && dur <= 6) {
                bootDurationSec = dur;
                prefs.putUChar("boot_dur", bootDurationSec);
                Serial.printf("[SETTINGS] Boot Duration: %d s\n", dur);
            }
        }
        // 6. Test/Preview Boot Splash: "BOOT:PREVIEW"
        else if (cmd == "BOOT:PREVIEW") {
            Serial.println("[SETTINGS] Previewing Boot Splash...");
            playBootSplash();
        }
        // 7. Brightness Value: "10".."255"
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

        // 1. Single Live Image Header: 0xAA 0x55 [len_hi] [len_lo]
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

        // 2. Persistent Boot Image Header: 0xAA 0x99 [len_hi] [len_lo]
        if (len >= 4 && bytes[0] == 0xAA && bytes[1] == 0x99) {
            bootBytesExpected = (bytes[2] << 8) | bytes[3];
            bootBytesReceived = 0;
            bootUploadType = 1;
            isBootUploading = true;
            LittleFS.remove("/boot_splash.jpg");
            bootFile = LittleFS.open("/boot_splash.jpg", "w");
            if (len > 4 && bootFile) {
                size_t payload = len - 4;
                bootFile.write(bytes + 4, payload);
                bootBytesReceived += payload;
            }
            Serial.printf("[LITTLEFS] Receiving Boot Image (%d bytes)...\n", (int)bootBytesExpected);
            return;
        }

        // 3. Persistent Boot Video Header: 0xBB 0x99 [total_frames] [fps]
        if (len >= 4 && bytes[0] == 0xBB && bytes[1] == 0x99) {
            bootUploadType = 2;
            isBootUploading = true;
            LittleFS.remove("/boot_anim.bin");
            bootFile = LittleFS.open("/boot_anim.bin", "w");
            if (bootFile) {
                bootFile.write(bytes[2]); // total_frames
                bootFile.write(bytes[3]); // fps
            }
            Serial.printf("[LITTLEFS] Receiving Boot Video (%d frames @ %d FPS)...\n", bytes[2], bytes[3]);
            return;
        }

        // 4. Live Video Init Header: 0xBB 0x66 [total_frames] [fps]
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

        // 5. Video Frame Header (Live or Boot): 0xCC [0x77|0x99] [frame_idx] [len_hi] [len_lo]
        if (len >= 5 && bytes[0] == 0xCC) {
            incomingFrameIdx = bytes[2];
            incomingFrameExpected = (bytes[3] << 8) | bytes[4];
            incomingFrameReceived = 0;

            if (bytes[1] == 0x99 && bootFile) {
                // Write 2-byte frame length header to LittleFS
                bootFile.write(bytes[3]);
                bootFile.write(bytes[4]);
                size_t payload = len - 5;
                if (payload > 0) {
                    bootFile.write(bytes + 5, payload);
                    incomingFrameReceived += payload;
                }
            } else if (bytes[1] == 0x77) {
                // Live Video buffer
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
            }
            return;
        }

        // 6. Finish / Play Header: 0xDD [0x88|0x99]
        if (len >= 2 && bytes[0] == 0xDD) {
            if (bytes[1] == 0x99) {
                // Finalize Boot Media Upload
                if (bootFile) {
                    bootFile.close();
                }
                isBootUploading = false;
                bootSplashType = (bootUploadType == 1) ? BOOT_CUSTOM_IMAGE : BOOT_CUSTOM_ANIM;
                prefs.putUChar("boot_type", (uint8_t)bootSplashType);
                Serial.printf("[LITTLEFS] Boot Media Saved! Type=%d. Previewing...\n", (int)bootSplashType);
                playBootSplash();
            } else if (bytes[1] == 0x88) {
                // Live Video Play
                isVideoPlaying = true;
                currentVideoFrame = 0;
                currentMode = MODE_STREAM_MEDIA;
                lastVideoFrameTime = millis();
                lastActivityTime = millis();
                Serial.printf("[STREAM] Playback started (%d frames @ %d FPS)!\n", totalVideoFrames, videoTargetFps);
            }
            return;
        }

        // 7. Append Boot Upload Chunk
        if (isBootUploading && bootFile) {
            bootFile.write(bytes, len);
            if (bootUploadType == 1) {
                bootBytesReceived += len;
                if (bootBytesReceived >= bootBytesExpected) {
                    bootFile.close();
                    isBootUploading = false;
                    bootSplashType = BOOT_CUSTOM_IMAGE;
                    prefs.putUChar("boot_type", (uint8_t)bootSplashType);
                    Serial.println("[LITTLEFS] Boot Image Upload Complete! Previewing...");
                    playBootSplash();
                }
            }
            return;
        }

        // 8. Append Live Video Frame Chunk
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

        // 9. Append Live Single Image Chunk
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
// VISUAL TOUCH RIPPLE & BLE NOTIFICATION TRIGGER
// =========================================================================
void triggerTouchVisual(const String& label, uint16_t color, uint32_t durationMs, const char* bleState) {
    touchVisualActive = true;
    touchVisualText = label;
    touchVisualColor = color;
    touchVisualEndTime = millis() + durationMs;

    if (bleConnected && pCharSet && bleState) {
        pCharSet->setValue(bleState);
        pCharSet->notify();
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
        triggerTouchVisual("TOUCH ⚡", 0x07FF, 300, "TOUCH:DOWN");
    }
    else if (!rawTouch && isTouching) {
        isTouching = false;
        uint32_t pressDuration = now - touchStartTime;
        lastActivityTime = now;
        triggerTouchVisual("IDLE", 0x8410, 100, "TOUCH:UP");

        if (pressDuration >= 450) {
            memePet.triggerHold();
            currentMode = MODE_CYBERPET;
            triggerTouchVisual("SHY LOVE ❤️", 0xF81F, 1000, "TOUCH:HOLD");
            if (bleConnected && pCharPet) {
                char emoChar[2] = { (char)('0' + (int)EMOTION_SHY), '\0' };
                pCharPet->setValue(emoChar);
                pCharPet->notify();
            }
            tapCount = 0;
        } else {
            if (tapCount == 0) {
                tapCount = 1;
                lastTapReleaseTime = now;
                memePet.triggerTap();
                triggerTouchVisual("POKE 👆", 0x07FF, 600, "TOUCH:POKE");
            } else if (tapCount == 1 && (now - lastTapReleaseTime) <= 350) {
                tapCount = 0;
                memePet.triggerDoubleTap();
                currentMode = MODE_CYBERPET;
                triggerTouchVisual("REACT ⚡", 0xFFE0, 800, "TOUCH:DOUBLE");
                if (bleConnected && pCharPet) {
                    char emoChar[2] = { (char)('0' + (int)memePet.currentEmotion), '\0' };
                    pCharPet->setValue(emoChar);
                    pCharPet->notify();
                }
            }
        }
    }

    if (tapCount == 1 && (now - lastTapReleaseTime) > 350) {
        tapCount = 0;
    }
}

// =========================================================================
// DEEP SLEEP WITH HARDWARE PIN HOLD (PURE ZERO-GLOW BACKLIGHT SHUTDOWN)
// =========================================================================
void enterDeepSleep() {
    Serial.println("[POWER] Entering Deep Sleep. Clamping backlight LOW...");

    tft.setBrightness(0);
    tft.sleep();

    digitalWrite(PIN_TFT_BL, LOW);
    pinMode(PIN_TFT_BL, OUTPUT);
    gpio_hold_en((gpio_num_t)PIN_TFT_BL);
    gpio_deep_sleep_hold_en();

    esp_deep_sleep_enable_gpio_wakeup(1ULL << PIN_TOUCH, ESP_GPIO_WAKEUP_GPIO_HIGH);

    Serial.flush();
    esp_deep_sleep_start();
}

// =========================================================================
// BOOT SPLASH SYSTEM (CUSTOM IMAGE, CUSTOM VIDEO, JOYBOY INTRO, OR INSTANT)
// =========================================================================
void playBootSplash() {
    if (bootSplashType == BOOT_INSTANT) {
        Serial.println("[BOOT] Instant boot requested. Skipping splash.");
        return;
    }

    // 1. Custom Image Boot Splash (/boot_splash.jpg from LittleFS)
    if (bootSplashType == BOOT_CUSTOM_IMAGE && LittleFS.exists("/boot_splash.jpg")) {
        File f = LittleFS.open("/boot_splash.jpg", "r");
        if (f) {
            size_t sz = f.size();
            uint8_t* buf = (uint8_t*)malloc(sz);
            if (buf) {
                f.read(buf, sz);
                f.close();
                Serial.printf("[BOOT] Rendering /boot_splash.jpg (%d bytes)...\n", (int)sz);
                canvas.fillScreen(TFT_BLACK);
                canvas.drawJpg(buf, sz, 0, 0, 240, 240);
                canvas.pushSprite(0, 0);
                free(buf);
                delay(bootDurationSec * 1000);
                return;
            }
            f.close();
        }
    }

    // 2. Custom Video Boot Splash (/boot_anim.bin from LittleFS)
    if (bootSplashType == BOOT_CUSTOM_ANIM && LittleFS.exists("/boot_anim.bin")) {
        File f = LittleFS.open("/boot_anim.bin", "r");
        if (f) {
            uint8_t totalFrames = f.read();
            uint8_t fps = f.read();
            if (fps == 0 || fps > 60) fps = 25;
            uint32_t frameDelay = 1000 / fps;
            Serial.printf("[BOOT] Playing /boot_anim.bin (%d frames @ %d FPS)...\n", totalFrames, fps);

            uint32_t endTime = millis() + (bootDurationSec * 1000);
            while (millis() < endTime && f.available() > 2) {
                f.seek(2);
                for (int i = 0; i < totalFrames && f.available() > 2; i++) {
                    uint8_t hi = f.read();
                    uint8_t lo = f.read();
                    size_t fLen = (hi << 8) | lo;
                    if (fLen == 0 || fLen > 15000 || fLen > (size_t)f.available()) break;
                    uint8_t* fBuf = (uint8_t*)malloc(fLen);
                    if (fBuf) {
                        f.read(fBuf, fLen);
                        canvas.drawJpg(fBuf, fLen, 0, 0, 240, 240);
                        canvas.pushSprite(0, 0);
                        free(fBuf);
                    } else {
                        f.seek(f.position() + fLen);
                    }
                    delay(frameDelay);
                }
            }
            f.close();
            return;
        }
    }

    // 3. Default Built-in Joyboy Cyber Boot Intro
    Serial.println("[BOOT] Playing Built-in Joyboy Cyber Intro...");
    uint32_t startIntro = millis();
    int pulse = 0;
    while (millis() - startIntro < (uint32_t)(bootDurationSec * 1000)) {
        canvas.fillScreen(0x0000);
        pulse = (pulse + 4) % 360;
        float rad = pulse * 0.0174533f;
        int ringR = 90 + (int)(sin(rad) * 6);

        // Cyber Grid Lines
        canvas.drawFastHLine(20, 120, 200, 0x18E3);
        canvas.drawFastVLine(120, 20, 200, 0x18E3);

        // Cyber Concentric Glowing Rings
        canvas.drawCircle(120, 120, ringR, 0x07FF);
        canvas.drawCircle(120, 120, ringR - 2, 0x03EF);
        canvas.drawCircle(120, 120, 48, 0xFD20);

        // Neon Border Frame
        canvas.drawRoundRect(6, 6, 228, 228, 8, 0x07FF);
        canvas.drawRoundRect(8, 8, 224, 224, 6, 0x0210);

        // Coffee Icon in center
        EmojiRenderer::drawEmoji(&canvas, EMOJI_COFFEE, 108, 64);

        // Glowing Typography
        canvas.setTextColor(0x07FF, 0x0000);
        canvas.setTextSize(2);
        canvas.drawCenterString("JOYBOY", 120, 112);

        canvas.setTextColor(0xFD20, 0x0000);
        canvas.setTextSize(2);
        canvas.drawCenterString("COFFEE", 120, 134);

        canvas.setTextColor(0x07E0, 0x0000);
        canvas.setTextSize(1);
        canvas.drawCenterString("DIGI KEYCHAIN v4.2", 120, 168);

        // Cyber Progress Bar
        float pct = (float)(millis() - startIntro) / (float)(bootDurationSec * 1000);
        if (pct > 1.0f) pct = 1.0f;
        canvas.drawRoundRect(40, 192, 160, 8, 3, 0x07FF);
        canvas.fillRect(42, 194, (int)(156 * pct), 4, 0x07E0);

        canvas.pushSprite(0, 0);
        delay(30);
    }
}

// =========================================================================
// INITIAL SETUP
// =========================================================================
void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println("\n=== DIGI KEYCHAIN ENGINE v4.2 STARTUP ===");

    // 1. Release Deep Sleep GPIO Hold
    gpio_hold_dis((gpio_num_t)PIN_TFT_BL);
    pinMode(PIN_TFT_BL, OUTPUT);
    digitalWrite(PIN_TFT_BL, HIGH);

    pinMode(PIN_TOUCH, INPUT);
    analogSetPinAttenuation(PIN_BAT_ADC, ADC_11db);
    pinMode(PIN_BAT_ADC, INPUT);

    // Initialize LittleFS
    if (!LittleFS.begin(true)) {
        Serial.println("[LITTLEFS] Mount Failed!");
    } else {
        Serial.println("[LITTLEFS] Mounted successfully.");
    }

    // Load Persistent Preferences
    prefs.begin("digi_keychain", false);
    defaultMode = (SystemMode)prefs.getUChar("def_mode", (uint8_t)MODE_CYBERPET);
    currentMode = defaultMode;
    memePet.defaultEmotion = (MemeEmotion)prefs.getUChar("def_emo", (uint8_t)EMOTION_LUFFY);
    memePet.currentEmotion = memePet.defaultEmotion;
    bootSplashType = (BootSplashType)prefs.getUChar("boot_type", (uint8_t)BOOT_JOYBOY_INTRO);
    bootDurationSec = prefs.getUChar("boot_dur", 2);
    screenBrightness = prefs.getUChar("br", 240);
    screenRotation = prefs.getUChar("rot", 3);
    sleepTimeoutMs = prefs.getUInt("sleep", 60000);
    customMessage = prefs.getString("msg", "I AM JOY BOY COFFEE :coffee: :fire:");

    // 2. Initialize NimBLE Bluetooth
    Serial.println("[BLE] Initializing NimBLE stack...");
    NimBLEDevice::init("DIGI_KEYCHAIN");
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);

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

    pCharSet = pService->createCharacteristic(CHAR_SETTINGS_UUID, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY);
    pCharSet->setCallbacks(new SettingsCallback());

    pCharBattery = pService->createCharacteristic(CHAR_BATTERY_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);

    auto pCharStream = pService->createCharacteristic(CHAR_STREAM_UUID, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR);
    pCharStream->setCallbacks(new StreamCallback());

    pService->start();

    // Configure Advertising: Device Name in Primary Ad, 128-bit Service in Scan Response
    NimBLEAdvertising* pAdv = NimBLEDevice::getAdvertising();
    pAdv->setScanResponse(true);
    pAdv->addServiceUUID(SERVICE_UUID);
    pAdv->start();
    Serial.println("[BLE] Advertising started as DIGI_KEYCHAIN (6e400001-b5a3-f393-e0a9-e50e24dcca9e)");

    // 3. Initialize Display
    Serial.println("[DISPLAY] Initializing ST7789 display...");
    tft.init();
    tft.setRotation(screenRotation);
    tft.setBrightness(screenBrightness);

    // 4. Create Double Buffer Canvas Sprite
    canvas.setColorDepth(16);
    canvas.createSprite(240, 240);

    // 5. Cold Boot Splash vs Deep Sleep Wakeup
    esp_sleep_wakeup_cause_t wakeupReason = esp_sleep_get_wakeup_cause();
    if (wakeupReason == ESP_SLEEP_WAKEUP_UNDEFINED) {
        playBootSplash();
    } else {
        Serial.printf("[BOOT] Woke from Deep Sleep (%d). Skipping splash.\n", (int)wakeupReason);
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

    // Auto Sleep Check (Only when NOT connected over BLE)
    if (!bleConnected && sleepTimeoutMs > 0 && (millis() - lastActivityTime) > sleepTimeoutMs) {
        enterDeepSleep();
    }

    // Periodic Battery Telemetry Update (every 5 seconds)
    static uint32_t lastBatCheck = 0;
    if (millis() - lastBatCheck >= 5000) {
        lastBatCheck = millis();
        updateBatteryTelemetry();
    }

    // Render Active Mode into Canvas
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

        case MODE_TEXT_SCROLL: {
            canvas.fillScreen(TFT_BLACK);
            canvas.drawRoundRect(2, 2, 236, 236, 8, 0x07FF);
            canvas.drawRoundRect(4, 4, 232, 232, 6, 0x18E3);

            canvas.setTextColor(0x07FF, TFT_BLACK);
            canvas.setTextSize(1);
            canvas.drawCenterString("MARQUEE BROADCAST", 120, 18);

            int endX = EmojiRenderer::renderTextWithEmojis(&canvas, customMessage, scrollX, 96, 3, 0xFFFF, 0x0000);
            scrollX -= 3;
            if (endX < 0) {
                scrollX = 240;
            }

            canvas.setTextColor(0x8410, TFT_BLACK);
            canvas.setTextSize(1);
            canvas.drawCenterString("DIGI KEYCHAIN", 120, 205);
            break;
        }

        case MODE_STREAM_MEDIA: {
            if (isVideoPlaying && totalVideoFrames > 0) {
                uint32_t now = millis();
                uint32_t frameInterval = 1000 / videoTargetFps;
                if (now - lastVideoFrameTime >= frameInterval) {
                    lastVideoFrameTime = now;
                    if (frameLengths[currentVideoFrame] > 0) {
                        uint8_t* fData = videoPool + frameOffsets[currentVideoFrame];
                        size_t fLen = frameLengths[currentVideoFrame];
                        canvas.drawJpg(fData, fLen, 0, 0, 240, 240);
                    }
                    currentVideoFrame = (currentVideoFrame + 1) % totalVideoFrames;
                }
            } else if (newMediaFrameReady && streamBytesReceived > 0) {
                canvas.drawJpg(streamBuffer, streamBytesReceived, 0, 0, 240, 240);
            }
            break;
        }
    }

    // =====================================================================
    // ON-SCREEN TOUCH VISUALIZER OVERLAY
    // =====================================================================
    if (isTouching || millis() < touchVisualEndTime) {
        int pulseR = (millis() / 40) % 10 + 4;
        canvas.drawCircle(222, 18, pulseR, touchVisualColor);
        canvas.fillCircle(222, 18, 4, touchVisualColor);

        canvas.fillRoundRect(134, 6, 82, 22, 4, 0x0000);
        canvas.drawRoundRect(134, 6, 82, 22, 4, touchVisualColor);
        canvas.setTextColor(touchVisualColor, 0x0000);
        canvas.setTextSize(1);
        canvas.drawCenterString(touchVisualText.c_str(), 175, 13);
    }

    // Push Double Buffer to Physical ST7789 Screen
    canvas.pushSprite(0, 0);

    // Dynamic frame pacing
    if (currentMode == MODE_STREAM_MEDIA && isVideoPlaying) {
        delay(5);
    } else {
        delay(25);
    }
}
