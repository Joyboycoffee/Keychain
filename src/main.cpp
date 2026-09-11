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
uint32_t       sleepTimeoutMs   = 0; // Default to NEVER sleep for battery discharge runs!

// Scrolling Marquee Message State
String customMessage = "I AM JOY BOY COFFEE :coffee: :fire:";
int scrollX = 240;

// Battery Telemetry & Discharge Run Logger
float    currentBatVoltage   = 4.20f;
int      currentBatPercent   = 100;
bool     isUsbPower          = true;
uint32_t sessionStartTimeMs  = 0;
uint16_t sessionStartMv      = 4200;
uint32_t lastSavedRunSec     = 0;
uint16_t lastSavedStartMv    = 4200;
uint16_t lastSavedEndMv      = 4200;
bool     lastSavedWasUsb     = false;
uint32_t totalRunCycles      = 1;
uint32_t allTimeRunSec       = 0;

// Power & BLE State Management
bool     bleActive           = false;
bool     bleConnected        = false;
uint32_t bleStartTimeMs      = 0;
bool     bleToggleFired      = false;

// Touch Interrupt & State Engine (Zero-Latency Hardware ISR)
volatile bool     isrTouchDown       = false;
volatile uint32_t isrDownTime        = 0;
volatile uint32_t isrUpTime          = 0;
volatile uint32_t isrTapCount        = 0;
volatile uint32_t isrLastTapEndTime  = 0;
volatile uint32_t isrLastEdgeTime    = 0;

uint32_t    lastActivityTime    = 0;
bool        holdTriggered       = false;
MemeEmotion preHoldEmotion      = EMOTION_LUFFY;
bool        isTemporaryLove     = false;
uint32_t    loveStartTime       = 0;

// On-Screen Touch Visualizer
bool     touchVisualActive   = false;
String   touchVisualText     = "TOUCH";
uint16_t touchVisualColor    = 0x07FF;
uint32_t touchVisualEndTime  = 0;

// Dynamic Image / Video Stream Buffers
#define STREAM_CHUNK_BUFFER 20480
uint8_t* pStreamBuf          = nullptr;
size_t   streamBytesReceived = 0;
size_t   expectedStreamBytes = 0;
bool     newMediaFrameReady  = false;

// 25 FPS Video / GIF Dynamic Stream Pool
#define MAX_VIDEO_FRAMES 30
#define VIDEO_POOL_MAX_SIZE 65000
uint8_t* pVideoPool          = nullptr;
size_t   videoPoolWriteOffset = 0;
size_t   frameOffsets[MAX_VIDEO_FRAMES];
size_t   frameLengths[MAX_VIDEO_FRAMES];
int      totalVideoFrames     = 0;
int      currentVideoFrame    = 0;
uint8_t  videoTargetFps       = 25;
bool     isVideoPlaying       = false;
uint32_t lastVideoFrameTime   = 0;
int      incomingFrameIdx     = -1;
size_t   incomingFrameExpected= 0;
size_t   incomingFrameReceived= 0;

// Boot Media Upload State
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
void saveBatteryDischargeLog();
void playBootSplash();
void triggerTouchVisual(const String& label, uint16_t color, uint32_t durationMs, const char* bleState);
void blinkDebugLed(int count, int delayMs = 150);
void startBLE(bool notifyVisual = true);
void stopBLE(bool notifyVisual = true);
void enterDeepSleep();

// =========================================================================
// BLE CALLBACKS
// =========================================================================
class ServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer) {
        bleConnected = true;
        lastActivityTime = millis();
        Serial.printf("\n[BLE] Client Connected! Free Heap: %u bytes\n", (unsigned int)ESP.getFreeHeap());
    }
    void onDisconnect(NimBLEServer* pServer) {
        bleConnected = false;
        lastActivityTime = millis();
        Serial.printf("\n[BLE] Client Disconnected. Free Heap: %u bytes\n", (unsigned int)ESP.getFreeHeap());
        if (bleActive) {
            bleStartTimeMs = millis(); // Refresh pairing window so user can refresh web page and reconnect!
            NimBLEDevice::startAdvertising();
        }
    }
};

class ModeCallback : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pChar) {
        std::string val = pChar->getValue();
        if (val.length() > 0) {
            int m = val[0] - '0';
            if (m >= 0 && m <= 5) {
                currentMode = (SystemMode)m;
                if (currentMode != MODE_STREAM_MEDIA) {
                    isVideoPlaying = false;
                }
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
        // 2. Sleep Timeout Command: "SLEEP:0" (0 = Never, 30 = 30s)
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
        // 7. Get Battery Discharge Log: "BAT:GET_LOG"
        else if (cmd == "BAT:GET_LOG") {
            uint32_t curRunSec = (millis() - sessionStartTimeMs) / 1000;
            char logMsg[128];
            // Format: "BAT_LOG|last_sec|last_start_mv|last_end_mv|cur_sec|cur_mv|usb|cycles|all_time_sec"
            snprintf(logMsg, sizeof(logMsg), "BAT_LOG|%u|%u|%u|%u|%u|%d|%u|%u",
                     lastSavedRunSec, lastSavedStartMv, lastSavedEndMv,
                     curRunSec, (uint16_t)(currentBatVoltage * 1000),
                     isUsbPower ? 1 : 0, totalRunCycles, allTimeRunSec + curRunSec);
            if (pCharSet) {
                pCharSet->setValue(std::string(logMsg));
                pCharSet->notify();
            }
            Serial.printf("[BATTERY] Sent Log: %s\n", logMsg);
        }
        // 8. Reset Battery Log: "BAT:RESET_LOG"
        else if (cmd == "BAT:RESET_LOG") {
            lastSavedRunSec = 0;
            lastSavedStartMv = (uint16_t)(currentBatVoltage * 1000);
            lastSavedEndMv = lastSavedStartMv;
            totalRunCycles = 1;
            allTimeRunSec = 0;
            prefs.putUInt("l_run", 0);
            prefs.putUShort("l_start", lastSavedStartMv);
            prefs.putUShort("l_end", lastSavedEndMv);
            prefs.putUInt("t_cycles", 1);
            prefs.putUInt("all_sec", 0);
            if (pCharSet) {
                pCharSet->setValue(std::string("BAT_LOG|0|0|0|0|0|0|1|0"));
                pCharSet->notify();
            }
            Serial.println("[BATTERY] Battery log reset.");
        }
        // 9. Save All Changes to Flash Memory: "SAVE_CONFIG" or "SAVE_CHANGES"
        else if (cmd == "SAVE_CONFIG" || cmd == "SAVE_CHANGES") {
            defaultMode = currentMode;
            memePet.defaultEmotion = memePet.currentEmotion;
            prefs.putUChar("def_mode", (uint8_t)defaultMode);
            prefs.putUChar("def_emo", (uint8_t)memePet.defaultEmotion);
            prefs.putUChar("br", screenBrightness);
            prefs.putUChar("rot", screenRotation);
            prefs.putUInt("sleep", sleepTimeoutMs);
            prefs.putUChar("boot_type", (uint8_t)bootSplashType);
            prefs.putUChar("boot_dur", bootDurationSec);
            prefs.putString("msg", customMessage);
            if (pCharSet) {
                pCharSet->setValue(std::string("SAVED:OK"));
                pCharSet->notify();
            }
            triggerTouchVisual("SAVED DEFAULTS 💾", 0x07E0, 1500, "SAVED:OK");
            Serial.println("[SETTINGS] Settings flashed to NVS memory as permanent default!");
        }
        // 10. Turn Off BLE Radio (Save Power): "BLE:OFF"
        else if (cmd == "BLE:OFF") {
            Serial.println("[SETTINGS] Web requested BLE power down.");
            if (pCharSet) {
                pCharSet->setValue(std::string("BLE:OFFLINE"));
                pCharSet->notify();
            }
            delay(150);
            stopBLE(true);
        }
        // 11. Enter Deep Sleep Immediately: "SYS:SLEEP"
        else if (cmd == "SYS:SLEEP") {
            Serial.println("[SETTINGS] Web requested immediate deep sleep.");
            enterDeepSleep();
        }
        // 12. Brightness Value: "10".."255"
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
            newMediaFrameReady = false;
            isVideoPlaying = false;
            if (!pStreamBuf) pStreamBuf = (uint8_t*)malloc(STREAM_CHUNK_BUFFER);
            if (pStreamBuf && len > 4) {
                size_t payload = len - 4;
                if (payload <= STREAM_CHUNK_BUFFER) {
                    memcpy(pStreamBuf, bytes + 4, payload);
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

        // 4. Live Video / GIF Stream Init Header: 0xBB 0x66 [total_frames] [fps]
        if (len >= 4 && bytes[0] == 0xBB && bytes[1] == 0x66) {
            totalVideoFrames = bytes[2];
            if (totalVideoFrames > MAX_VIDEO_FRAMES) totalVideoFrames = MAX_VIDEO_FRAMES;
            videoTargetFps = bytes[3] > 0 ? bytes[3] : 25;
            videoPoolWriteOffset = 0;
            currentVideoFrame = 0;
            isVideoPlaying = false;
            incomingFrameIdx = -1;
            
            if (!pVideoPool) pVideoPool = (uint8_t*)malloc(VIDEO_POOL_MAX_SIZE);
            Serial.printf("[STREAM] Video/GIF Stream Init: %d frames @ %d FPS (Buffer Allocated)\n", totalVideoFrames, videoTargetFps);
            return;
        }

        // 5. Video Frame Header (Live or Boot): 0xCC [0x77|0x99] [frame_idx] [len_hi] [len_lo]
        if (len >= 5 && bytes[0] == 0xCC) {
            incomingFrameIdx = bytes[2];
            incomingFrameExpected = (bytes[3] << 8) | bytes[4];
            incomingFrameReceived = 0;

            if (bytes[1] == 0x99 && bootFile) {
                // Boot animation to flash
                bootFile.write(bytes[3]);
                bootFile.write(bytes[4]);
                size_t payload = len - 5;
                if (payload > 0) {
                    bootFile.write(bytes + 5, payload);
                    incomingFrameReceived += payload;
                }
            } else if (bytes[1] == 0x77 && pVideoPool) {
                // Live video buffer
                if (incomingFrameIdx < MAX_VIDEO_FRAMES && (videoPoolWriteOffset + incomingFrameExpected) <= VIDEO_POOL_MAX_SIZE) {
                    frameOffsets[incomingFrameIdx] = videoPoolWriteOffset;
                    frameLengths[incomingFrameIdx] = 0;
                    size_t payload = len - 5;
                    if (payload > 0) {
                        memcpy(pVideoPool + videoPoolWriteOffset, bytes + 5, payload);
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
                Serial.printf("[STREAM] Video/GIF Playback Started (%d frames @ %d FPS)!\n", totalVideoFrames, videoTargetFps);
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
        if (incomingFrameIdx >= 0 && incomingFrameReceived < incomingFrameExpected && pVideoPool) {
            if (videoPoolWriteOffset + len <= VIDEO_POOL_MAX_SIZE) {
                memcpy(pVideoPool + videoPoolWriteOffset, bytes, len);
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
        if (expectedStreamBytes > 0 && streamBytesReceived < expectedStreamBytes && pStreamBuf) {
            if (streamBytesReceived + len <= STREAM_CHUNK_BUFFER) {
                memcpy(pStreamBuf + streamBytesReceived, bytes, len);
                streamBytesReceived += len;
            }
            if (streamBytesReceived >= expectedStreamBytes) {
                newMediaFrameReady = true;
                currentMode = MODE_STREAM_MEDIA;
                lastActivityTime = millis();
                Serial.printf("[STREAM] Image ready (%d bytes)!\n", (int)streamBytesReceived);
            }
        }
    }
};

// =========================================================================
// BATTERY VOLTAGE SENSING & DISCHARGE LOGGING (GPIO 0)
// =========================================================================
void saveBatteryDischargeLog() {
    uint32_t curRunSec = (millis() - sessionStartTimeMs) / 1000;
    uint16_t curEndMv = (uint16_t)(currentBatVoltage * 1000);
    
    // Save live session state into NVS preferences
    prefs.putUInt("l_run", curRunSec);
    prefs.putUShort("l_end", curEndMv);
    prefs.putBool("l_usb", isUsbPower);
    lastSavedRunSec = curRunSec;
    lastSavedEndMv = curEndMv;
    lastSavedWasUsb = isUsbPower;
}

void updateBatteryTelemetry() {
    // 16-sample oversampled average to eliminate noise and spikes
    uint32_t rawSum = 0;
    for (int i = 0; i < 16; i++) {
        rawSum += analogReadMilliVolts(PIN_BAT_ADC);
        delayMicroseconds(50);
    }
    float rawMv = (float)rawSum / 16.0f;
    
    // Calibrated divider multiplier: 2.16f accounts for 100k+100k resistor loading & ADC attenuation
    float vbat = (rawMv * 2.16f) / 1000.0f;
    
    if (vbat < 2.5f) {
        currentBatVoltage = 5.0f;
        currentBatPercent = 100;
        isUsbPower = true;
    } else {
        currentBatVoltage = vbat;
        isUsbPower = false;
        
        // Realistic multi-point LiPo discharge curve
        if (vbat >= 4.15f) currentBatPercent = 100;
        else if (vbat >= 4.00f) currentBatPercent = 85 + (int)((vbat - 4.00f) / 0.15f * 15.0f);
        else if (vbat >= 3.85f) currentBatPercent = 65 + (int)((vbat - 3.85f) / 0.15f * 20.0f);
        else if (vbat >= 3.75f) currentBatPercent = 45 + (int)((vbat - 3.75f) / 0.10f * 20.0f);
        else if (vbat >= 3.60f) currentBatPercent = 20 + (int)((vbat - 3.60f) / 0.15f * 25.0f);
        else if (vbat >= 3.40f) currentBatPercent = 5 + (int)((vbat - 3.40f) / 0.20f * 15.0f);
        else currentBatPercent = (int)((vbat - 3.00f) / 0.40f * 5.0f);
        
        if (currentBatPercent > 100) currentBatPercent = 100;
        if (currentBatPercent < 0) currentBatPercent = 0;
    }

    cyberHUD.setBattery(currentBatVoltage, currentBatPercent, isUsbPower);

    if (bleConnected && pCharBattery) {
        char bMsg[32];
        snprintf(bMsg, sizeof(bMsg), "%.2f|%d|%d", currentBatVoltage, currentBatPercent, isUsbPower ? 1 : 0);
        pCharBattery->setValue(std::string(bMsg));
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
        pCharSet->setValue(std::string(bleState));
        pCharSet->notify();
    }
}

// =========================================================================
// POWER & BLE MANAGEMENT HELPERS
// =========================================================================
void blinkDebugLed(int count, int delayMs) {
    pinMode(PIN_DEBUG_LED, OUTPUT);
    for (int i = 0; i < count; i++) {
        digitalWrite(PIN_DEBUG_LED, LOW);  // Turn ON (Active LOW on SuperMini)
        delay(delayMs);
        digitalWrite(PIN_DEBUG_LED, HIGH); // Turn OFF
        delay(delayMs);
    }
    digitalWrite(PIN_DEBUG_LED, HIGH);     // Keep OFF
}

void startBLE(bool notifyVisual) {
    if (bleActive) return;
    bleActive = true;
    setCpuFrequencyMhz(160);
    blinkDebugLed(2, 180); // Blink twice and stay OFF
    NimBLEAdvertising* pAdv = NimBLEDevice::getAdvertising();
    if (pAdv) pAdv->start();
    bleStartTimeMs = millis();
    lastActivityTime = millis();
    if (notifyVisual) triggerTouchVisual("BLE ON (35s) ⚡", 0x07FF, 2000, "BLE:ONLINE");
    Serial.println("[BLE] BLE Radio Activated! 35s pairing window started. CPU @ 160MHz.");
}

void stopBLE(bool notifyVisual) {
    if (!bleActive && !bleConnected) return;
    bleActive = false;
    NimBLEAdvertising* pAdv = NimBLEDevice::getAdvertising();
    if (pAdv) pAdv->stop();
    if (bleConnected) {
        NimBLEDevice::getServer()->disconnect(0);
        bleConnected = false;
    }
    blinkDebugLed(1, 150); // Blink once
    setCpuFrequencyMhz(80); // Scale CPU clock down to 80 MHz to save power
    if (notifyVisual) triggerTouchVisual("BLE OFF 💤", 0x8410, 1500, "BLE:OFFLINE");
    Serial.println("[BLE] BLE Radio Deactivated! Standby mode. CPU @ 80MHz.");
}

// =========================================================================
// ZERO-LATENCY HARDWARE TOUCH ISR & GESTURE ENGINE
// =========================================================================
void IRAM_ATTR touchISR() {
    uint32_t now = millis();
    if (now - isrLastEdgeTime < 10) return; // 10ms spike rejection
    isrLastEdgeTime = now;

    bool pinHigh = (digitalRead(PIN_TOUCH) == HIGH);
    if (pinHigh) {
        if (!isrTouchDown) {
            isrTouchDown = true;
            isrDownTime = now;
        }
    } else {
        if (isrTouchDown) {
            isrTouchDown = false;
            isrUpTime = now;
            uint32_t dur = isrUpTime - isrDownTime;
            if (dur >= 15 && dur < 1200) {
                isrTapCount++;
                isrLastTapEndTime = now;
            }
        }
    }
}

void processTouch() {
    uint32_t now = millis();
    bool isDown = isrTouchDown || (digitalRead(PIN_TOUCH) == HIGH);

    // 1. DOUBLE TAP DETECTION (Instant switch on 2nd tap down OR 2 taps completed)
    if (isrTapCount >= 2 || (isrTapCount == 1 && isDown && (now - isrLastTapEndTime <= 650) && (now - isrLastTapEndTime >= 15))) {
        isrTapCount = 0;
        lastActivityTime = now;
        holdTriggered = false;
        bleToggleFired = false;
        isTemporaryLove = false;

        int next = ((int)memePet.currentEmotion + 1) % 7;
        memePet.setEmotion((MemeEmotion)next);
        memePet.defaultEmotion = (MemeEmotion)next; // Persist mascot
        currentMode = MODE_CYBERPET;
        isVideoPlaying = false;

        const char* emoNames[] = { "LUFFY ⚡", "SHY LOVE 👉👈", "GIGGLE CAT 😸", "SAD BANANA 🍌", "UMARU CRY 😭", "ANGRY CAT 😾", "BUNNY 🐰" };
        triggerTouchVisual(emoNames[next], 0xFFE0, 1000, "TOUCH:DOUBLE");

        if (bleConnected && pCharPet) {
            char emoChar[2] = { (char)('0' + (int)next), '\0' };
            pCharPet->setValue(std::string(emoChar));
            pCharPet->notify();
        }
        Serial.printf("[TOUCH] Instant Double-Tap! -> Switched Mascot: %d (%s)\n", next, emoNames[next]);
        return;
    }

    // 2. CONTINUOUS HOLD (2.0s for Shy Love, 10.0s for BLE Toggle)
    if (isDown && isrTapCount == 0) {
        if (isrDownTime == 0) {
            isrDownTime = now;
        }
        uint32_t holdDuration = now - isrDownTime;

        // 10-Second Long Hold -> TOGGLE BLE ON/OFF!
        if (holdDuration >= 10000 && !bleToggleFired) {
            bleToggleFired = true;
            holdTriggered = true;
            isrTapCount = 0;
            lastActivityTime = now;

            if (!bleActive) {
                startBLE(true);
            } else {
                stopBLE(true);
            }
            return;
        }

        // 2.0-Second Hold -> Shy Love ❤️
        if (holdDuration >= 2000 && holdDuration < 9000 && !holdTriggered && !bleToggleFired) {
            holdTriggered = true;
            isrTapCount = 0;
            lastActivityTime = now;

            preHoldEmotion = memePet.currentEmotion;
            isTemporaryLove = true;
            loveStartTime = now;

            memePet.setEmotion(EMOTION_SHY);
            currentMode = MODE_CYBERPET;
            isVideoPlaying = false;
            triggerTouchVisual("SHY LOVE ❤️", 0xF81F, 5000, "TOUCH:HOLD");

            if (bleConnected && pCharPet) {
                char emoChar[2] = { (char)('0' + (int)EMOTION_SHY), '\0' };
                pCharPet->setValue(std::string(emoChar));
                pCharPet->notify();
            }
            Serial.printf("[TOUCH] 2.0s Hold -> Shy Love! (Reverting to %d in 5s)\n", (int)preHoldEmotion);
            return;
        }
    } else if (!isDown) {
        isrDownTime = 0;
    }

    if (!isDown) {
        if (holdTriggered || bleToggleFired) {
            holdTriggered = false;
            bleToggleFired = false;
            isrTapCount = 0;
        }
    }

    // 3. SINGLE TAP TIMEOUT (Poke Triggered)
    // If 1 tap was recorded, finger is lifted, and 450ms have passed without a second tap:
    if (isrTapCount == 1 && !isDown && (now - isrLastTapEndTime > 450)) {
        isrTapCount = 0;
        lastActivityTime = now;
        if (!holdTriggered && !bleToggleFired) {
            memePet.triggerTap();
            triggerTouchVisual("POKE 👆", 0x07FF, 700, "TOUCH:POKE");
            Serial.println("[TOUCH] Single Tap Confirmed -> POKE 👆");
        }
    }

    // 4. AUTO-REVERT FROM SHY LOVE AFTER 5 SECONDS BACK TO ORIGINAL PHOTO
    if (isTemporaryLove && (now - loveStartTime >= 5000)) {
        isTemporaryLove = false;
        memePet.setEmotion(preHoldEmotion);
        memePet.defaultEmotion = preHoldEmotion;
        if (bleConnected && pCharPet) {
            char emoChar[2] = { (char)('0' + (int)preHoldEmotion), '\0' };
            pCharPet->setValue(std::string(emoChar));
            pCharPet->notify();
        }
        const char* emoNames[] = { "LUFFY ⚡", "SHY LOVE 👉👈", "GIGGLE CAT 😸", "SAD BANANA 🍌", "UMARU CRY 😭", "ANGRY CAT 😾", "BUNNY 🐰" };
        triggerTouchVisual(emoNames[preHoldEmotion], 0x07FF, 700, "TOUCH:REVERT");
        Serial.printf("[TOUCH] 5s elapsed -> Reverted to %d (%s)\n", (int)preHoldEmotion, emoNames[preHoldEmotion]);
    }
}

void enterDeepSleep() {
    Serial.println("[POWER] Entering Deep Sleep...");

    saveBatteryDischargeLog();

    if (bleActive || bleConnected) {
        stopBLE(false);
    }

    // Wait until touch sensor is completely released before sleeping!
    uint32_t waitRelease = millis();
    while (digitalRead(PIN_TOUCH) == HIGH && (millis() - waitRelease < 2500)) {
        delay(20);
    }
    delay(50); // Debounce settling delay

    tft.setBrightness(0);
    tft.sleep();

    digitalWrite(PIN_TFT_BL, LOW);
    pinMode(PIN_TFT_BL, OUTPUT);
    gpio_hold_en((gpio_num_t)PIN_TFT_BL);
    gpio_deep_sleep_hold_en();

    detachInterrupt(digitalPinToInterrupt(PIN_TOUCH));
    esp_deep_sleep_enable_gpio_wakeup(1ULL << PIN_TOUCH, ESP_GPIO_WAKEUP_GPIO_HIGH);

    Serial.flush();
    esp_deep_sleep_start();
}

// =========================================================================
// BOOT SPLASH SYSTEM
// =========================================================================
void playBootSplash() {
    if (bootSplashType == BOOT_INSTANT) {
        Serial.println("[BOOT] Instant boot requested.");
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
                    if (fLen == 0 || fLen > 10000 || fLen > (size_t)f.available()) break;
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
        pulse = (pulse + 5) % 360;
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
        canvas.drawCenterString("DIGI KEYCHAIN v4.3", 120, 168);

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
    delay(200);
    Serial.println("\n=== DIGI KEYCHAIN ENGINE v4.3 STARTUP ===");

    // 1. Release Deep Sleep GPIO Hold
    gpio_hold_dis((gpio_num_t)PIN_TFT_BL);
    pinMode(PIN_TFT_BL, OUTPUT);
    digitalWrite(PIN_TFT_BL, HIGH);

    pinMode(PIN_TOUCH, INPUT);
    attachInterrupt(digitalPinToInterrupt(PIN_TOUCH), touchISR, CHANGE);
    if (digitalRead(PIN_TOUCH) == HIGH) {
        isrTouchDown = true;
        isrDownTime = millis();
    }
    analogSetPinAttenuation(PIN_BAT_ADC, ADC_11db);
    pinMode(PIN_BAT_ADC, INPUT);

    // Initialize LittleFS
    if (!LittleFS.begin(true)) {
        Serial.println("[LITTLEFS] Mount Failed!");
    } else {
        Serial.println("[LITTLEFS] Mounted successfully.");
    }

    // Load Persistent Preferences & Battery Run Logger Stats
    prefs.begin("digi_keychain", false);
    defaultMode = (SystemMode)prefs.getUChar("def_mode", (uint8_t)MODE_CYBERPET);
    currentMode = defaultMode;
    memePet.defaultEmotion = (MemeEmotion)prefs.getUChar("def_emo", (uint8_t)EMOTION_LUFFY);
    memePet.currentEmotion = memePet.defaultEmotion;
    bootSplashType = (BootSplashType)prefs.getUChar("boot_type", (uint8_t)BOOT_JOYBOY_INTRO);
    bootDurationSec = prefs.getUChar("boot_dur", 2);
    screenBrightness = prefs.getUChar("br", 240);
    screenRotation = prefs.getUChar("rot", 3);
    sleepTimeoutMs = prefs.getUInt("sleep", 0); // 0 = Never sleep by default
    customMessage = prefs.getString("msg", "I AM JOY BOY COFFEE :coffee: :fire:");

    // Battery Discharge Logger State from NVS
    lastSavedRunSec  = prefs.getUInt("l_run", 0);
    lastSavedStartMv = prefs.getUShort("l_start", 4200);
    lastSavedEndMv   = prefs.getUShort("l_end", 4200);
    lastSavedWasUsb  = prefs.getBool("l_usb", false);
    totalRunCycles   = prefs.getUInt("t_cycles", 0) + 1;
    prefs.putUInt("t_cycles", totalRunCycles);

    sessionStartTimeMs = millis();
    updateBatteryTelemetry();
    sessionStartMv = (uint16_t)(currentBatVoltage * 1000);
    prefs.putUShort("l_start", sessionStartMv);

    Serial.printf("[BATTERY LOGGER] Last Run: %u sec | Start: %u mV -> End: %u mV | Cycle #%u\n",
                  lastSavedRunSec, lastSavedStartMv, lastSavedEndMv, totalRunCycles);

    // 2. Initialize Display & Canvas (115KB heap)
    Serial.println("[DISPLAY] Initializing ST7789 display...");
    tft.init();
    tft.setRotation(screenRotation);
    tft.setBrightness(screenBrightness);

    canvas.setColorDepth(16);
    if (!canvas.createSprite(240, 240)) {
        Serial.println("[DISPLAY] Sprite creation failed!");
    } else {
        Serial.println("[DISPLAY] 240x240 Sprite Buffer Created Successfully.");
    }

    // 3. Initialize NimBLE Bluetooth
    Serial.println("[BLE] Initializing NimBLE stack...");
    NimBLEDevice::init("DIGI_KEYCHAIN");
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);
    NimBLEDevice::setSecurityAuth(false, false, false);

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

    // Initial Startup: BLE starts in STANDBY (Radio OFF, CPU @ 80MHz to save max power)
    // Bluetooth only turns ON when user holds touch button for 10 seconds!
    bleActive = false;
    bleConnected = false;
    setCpuFrequencyMhz(80);
    digitalWrite(PIN_DEBUG_LED, HIGH); // Ensure LED is OFF
    Serial.printf("[BLE] Stack initialized in STANDBY (Radio OFF, CPU @ 80MHz). Hold touch for 10s to activate.\n");

    // 4. Cold Boot Splash vs Deep Sleep Wakeup
    esp_sleep_wakeup_cause_t wakeupReason = esp_sleep_get_wakeup_cause();
    if (wakeupReason == ESP_SLEEP_WAKEUP_UNDEFINED) {
        playBootSplash();
    } else {
        Serial.printf("[BOOT] Woke from Deep Sleep (%d). Skipping splash.\n", (int)wakeupReason);
    }

    lastActivityTime = millis();
    Serial.printf("[SYSTEM] Ready! Running active engine. Free Heap: %u bytes\n", (unsigned int)ESP.getFreeHeap());
}

// =========================================================================
// MAIN LOOP
// =========================================================================
void loop() {
    processTouch();

    // Auto BLE power-down if no connection after 35 seconds to save maximum battery
    if (bleActive && !bleConnected && (millis() - bleStartTimeMs > 35000)) {
        stopBLE(false);
    }

    // Keep resetting activity timer while connected so sleep timer only starts AFTER disconnect
    if (bleConnected) {
        lastActivityTime = millis();
    }

    // Auto Sleep Check (Only when BLE radio is completely OFF, disconnected, and user has been idle)
    if (!bleActive && !bleConnected && sleepTimeoutMs > 0 && (millis() - lastActivityTime > sleepTimeoutMs)) {
        enterDeepSleep();
    }

    // Periodic Battery Telemetry & Flash Log Update (every 20 seconds)
    static uint32_t lastBatCheck = 0;
    if (millis() - lastBatCheck >= 20000) {
        lastBatCheck = millis();
        updateBatteryTelemetry();
        saveBatteryDischargeLog();
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
            if (isVideoPlaying && totalVideoFrames > 0 && pVideoPool) {
                uint32_t now = millis();
                uint32_t frameInterval = 1000 / videoTargetFps;
                if (now - lastVideoFrameTime >= frameInterval) {
                    lastVideoFrameTime = now;
                    if (frameLengths[currentVideoFrame] > 0) {
                        uint8_t* fData = pVideoPool + frameOffsets[currentVideoFrame];
                        size_t fLen = frameLengths[currentVideoFrame];
                        canvas.drawJpg(fData, fLen, 0, 0, 240, 240);
                    }
                    currentVideoFrame = (currentVideoFrame + 1) % totalVideoFrames;
                }
            } else if (newMediaFrameReady && streamBytesReceived > 0 && pStreamBuf) {
                canvas.drawJpg(pStreamBuf, streamBytesReceived, 0, 0, 240, 240);
            }
            break;
        }
    }

    // =====================================================================
    // ON-SCREEN TOUCH VISUALIZER OVERLAY
    // =====================================================================
    if (millis() < touchVisualEndTime) {
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

    if (currentMode == MODE_STREAM_MEDIA && isVideoPlaying) {
        delay(5);
    } else {
        delay(25);
    }
}
