#include <Arduino.h>
#include <LovyanGFX.hpp>
#include <NimBLEDevice.h>
#include <Preferences.h>
#include <LittleFS.h>
#include "driver/gpio.h"
#include "esp_sleep.h"
#include "config.h"
#include "display_setup.h"
#include "emoji_renderer.h"
#include "meme_pet.h"
#include "robot_eyes.h"
#include "cyber_hud.h"
#include "matrix_rain.h"

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

// Touch Interrupt & State Engine (Zero-Latency Hardware ISR + 10s Release / 14s Cancel Engine)
volatile bool     isrTouchDown         = false;
volatile uint32_t isrDownTime          = 0;
volatile uint32_t isrUpTime            = 0;
volatile uint32_t isrTapCount          = 0;
volatile uint32_t isrLastTapEndTime    = 0;
volatile uint32_t isrLastEdgeTime      = 0;

uint32_t    lastActivityTime    = 0;
bool        shyLoveTriggered    = false;
bool        hold10sReady        = false;
bool        hold10sPrompted     = false;
bool        hold14sCancelled    = false;
bool        touchStuckLockout   = false;
uint32_t    touchLowStartTime   = 0;
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
                std::string rest = val.substr(barIdx + 1);
                size_t secondBar = rest.find('|');
                if (secondBar != std::string::npos) {
                    float temp = atof(rest.substr(0, secondBar).c_str());
                    String dateStr = String(rest.substr(secondBar + 1).c_str());
                    cyberHUD.setWeather(temp, "SYNCED");
                    cyberHUD.setDate(dateStr);
                } else {
                    float temp = atof(rest.c_str());
                    cyberHUD.setWeather(temp, "SYNCED");
                }
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
            prefs.putUChar("hud_lay", (uint8_t)cyberHUD.currentLayout);
            prefs.putBool("hud_sec", cyberHUD.showSeconds);
            prefs.putBool("hud_bat", cyberHUD.showBattery);
            prefs.putBool("hud_date", cyberHUD.showDate);
            prefs.putBool("hud_wave", cyberHUD.showWaveform);
            prefs.putBool("hud_text", cyberHUD.showCustomText);
            prefs.putString("hud_msg", cyberHUD.customMessage);
            prefs.putUChar("robot_mood", (uint8_t)robotEyes.currentStyle);
            prefs.putUChar("matrix_thm", (uint8_t)matrixRain.currentTheme);
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
        // 9. CyberHUD Config: "HUD_CFG:<layout>:<sec>:<bat>:<date>:<wave>:<text>"
        else if (cmd.startsWith("HUD_CFG:")) {
            String cfg = cmd.substring(8);
            int idx1 = cfg.indexOf(':');
            int idx2 = cfg.indexOf(':', idx1 + 1);
            int idx3 = cfg.indexOf(':', idx2 + 1);
            int idx4 = cfg.indexOf(':', idx3 + 1);
            int idx5 = cfg.indexOf(':', idx4 + 1);
            if (idx1 != -1) {
                int lay = cfg.substring(0, idx1).toInt();
                cyberHUD.setLayout(lay);
                prefs.putUChar("hud_lay", (uint8_t)cyberHUD.currentLayout);
                if (idx5 != -1) {
                    bool sec = cfg.substring(idx1 + 1, idx2).toInt() != 0;
                    bool bat = cfg.substring(idx2 + 1, idx3).toInt() != 0;
                    bool dt  = cfg.substring(idx3 + 1, idx4).toInt() != 0;
                    bool wav = cfg.substring(idx4 + 1, idx5).toInt() != 0;
                    bool txt = cfg.substring(idx5 + 1).toInt() != 0;
                    cyberHUD.setFlags(sec, bat, dt, wav, txt);
                    prefs.putBool("hud_sec", sec);
                    prefs.putBool("hud_bat", bat);
                    prefs.putBool("hud_date", dt);
                    prefs.putBool("hud_wave", wav);
                    prefs.putBool("hud_text", txt);
                }
                currentMode = MODE_CYBER_HUD;
                isVideoPlaying = false;
                Serial.printf("[SETTINGS] CyberHUD Configured: Layout=%d\n", cyberHUD.currentLayout);
            }
        }
        // 10. CyberHUD Custom Text: "HUD_MSG:<custom text>"
        else if (cmd.startsWith("HUD_MSG:")) {
            String msg = cmd.substring(8);
            cyberHUD.setCustomText(msg);
            prefs.putString("hud_msg", msg);
            currentMode = MODE_CYBER_HUD;
            isVideoPlaying = false;
            Serial.printf("[SETTINGS] CyberHUD Custom Text: %s\n", msg.c_str());
        }
        // 11. Robot Mood: "ROBOT_MOOD:<0..3>"
        else if (cmd.startsWith("ROBOT_MOOD:")) {
            int mood = cmd.substring(11).toInt();
            robotEyes.setStyle(mood);
            prefs.putUChar("robot_mood", (uint8_t)robotEyes.currentStyle);
            currentMode = MODE_ROBOT_EYES;
            isVideoPlaying = false;
            Serial.printf("[SETTINGS] Robot Mood: %d\n", robotEyes.currentStyle);
        }
        // 12. Matrix Theme: "MATRIX_THM:<0..3>"
        else if (cmd.startsWith("MATRIX_THM:")) {
            int thm = cmd.substring(11).toInt();
            matrixRain.setTheme(thm);
            prefs.putUChar("matrix_thm", (uint8_t)matrixRain.currentTheme);
            currentMode = MODE_MATRIX_RAIN;
            isVideoPlaying = false;
            Serial.printf("[SETTINGS] Matrix Theme: %d\n", matrixRain.currentTheme);
        }
        // 13. CyberHUD / Universal Shy Secret Message: "HUD_SHY:<custom text>"
        else if (cmd.startsWith("HUD_SHY:")) {
            String msg = cmd.substring(8);
            cyberHUD.setShyText(msg);
            robotEyes.setShyText(msg);
            matrixRain.setShyText(msg);
            prefs.putString("hud_shy", msg);
            Serial.printf("[SETTINGS] Synced Shy Secret Text: %s\n", msg.c_str());
        }
        // 14. Save All Changes to Flash Memory: "SAVE_CONFIG" or "SAVE_CHANGES"
        else if (cmd == "SAVE_CONFIG" || cmd == "SAVE_CHANGES") {
            defaultMode = currentMode;
            memePet.defaultEmotion = memePet.currentEmotion;
            prefs.putUChar("def_mode", (uint8_t)defaultMode);
            prefs.putUChar("def_emo", (uint8_t)memePet.defaultEmotion);
            prefs.putUChar("hud_lay", (uint8_t)cyberHUD.currentLayout);
            prefs.putBool("hud_sec", cyberHUD.showSeconds);
            prefs.putBool("hud_bat", cyberHUD.showBattery);
            prefs.putBool("hud_date", cyberHUD.showDate);
            prefs.putBool("hud_wave", cyberHUD.showWaveform);
            prefs.putBool("hud_text", cyberHUD.showCustomText);
            prefs.putString("hud_msg", cyberHUD.customMessage);
            prefs.putString("hud_shy", cyberHUD.customShyText);
            prefs.putUChar("robot_mood", (uint8_t)robotEyes.currentStyle);
            prefs.putUChar("matrix_thm", (uint8_t)matrixRain.currentTheme);
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
        // 14. Turn Off BLE Radio / Save Power / Sleep: "BLE:OFF"
        else if (cmd == "BLE:OFF") {
            Serial.println("[SETTINGS] Web requested BLE power down & deep sleep.");
            if (pCharSet) {
                pCharSet->setValue(std::string("BLE:OFFLINE"));
                pCharSet->notify();
            }
            delay(150);
            enterDeepSleep();
        }
        // 15. Enter Deep Sleep Immediately: "SYS:SLEEP"
        else if (cmd == "SYS:SLEEP") {
            Serial.println("[SETTINGS] Web requested immediate deep sleep.");
            enterDeepSleep();
        }
        // 16. Brightness Value: "10".."255"
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
    blinkDebugLed(1, 60); // Crisp single 60ms pulse (virtually 0 power), then stay OFF
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
    blinkDebugLed(1, 40); // Quick 40ms pulse
    setCpuFrequencyMhz(80); // Scale CPU clock down to 80 MHz to save power
    if (notifyVisual) triggerTouchVisual("BLE OFF 💤", 0x8410, 1500, "BLE:OFFLINE");
    Serial.println("[BLE] BLE Radio Deactivated! Standby mode. CPU @ 80MHz.");
}

// =========================================================================
// ZERO-LATENCY HARDWARE TOUCH ISR & GESTURE ENGINE
// =========================================================================
void IRAM_ATTR touchISR() {
    uint32_t now = millis();
    if (now - isrLastEdgeTime < 25) return; // 25ms hardware edge debounce
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
            uint32_t dur = (isrDownTime > 0) ? (isrUpTime - isrDownTime) : 0;
            isrDownTime = 0; // Clear hold timestamp on release
            // Require at least 55ms of continuous contact to qualify as a deliberate human tap
            if (dur >= 55 && dur < 600) {
                isrTapCount++;
                isrLastTapEndTime = now;
            }
        }
    }
}

void processTouch() {
    uint32_t now = millis();
    bool isDown = (digitalRead(PIN_TOUCH) == HIGH);

    // ---------------------------------------------------------
    // 1. PIN RELEASE & STABILIZATION TRACKING
    // ---------------------------------------------------------
    if (!isDown) {
        if (isrTouchDown) {
            isrTouchDown = false;
            isrUpTime = now;
        }
        if (touchLowStartTime == 0) {
            touchLowStartTime = now;
        }
        
        // Pin must stay continuously LOW for at least 250ms to fully clear lockout and hold states
        if (now - touchLowStartTime >= 250) {
            if (touchStuckLockout || hold14sCancelled) {
                touchStuckLockout = false;
                hold14sCancelled = false;
                Serial.println("[TOUCH] Sensor confirmed clear and released. Ready.");
            }
            hold10sReady = false;
            hold10sPrompted = false;
            shyLoveTriggered = false;
            isrDownTime = 0;
        }
    } else {
        touchLowStartTime = 0;
        // Only start a new hold timer if not currently locked out!
        if (!touchStuckLockout && !hold14sCancelled) {
            if (!isrTouchDown || isrDownTime == 0) {
                isrTouchDown = true;
                isrDownTime = now;
            }
        }
    }

    // ---------------------------------------------------------
    // 2. STUCK LOCKOUT SHIELD
    // If the sensor was held past 14s or was HIGH at boot, ignore ALL processing while down!
    // ---------------------------------------------------------
    if (touchStuckLockout || hold14sCancelled) {
        return;
    }

    // ---------------------------------------------------------
    // 3. 5-TAP DETECTION (Instant Switch Main System Mode ⚡)
    // ---------------------------------------------------------
    if (isrTapCount >= 5) {
        isrTapCount = 0;
        lastActivityTime = now;
        isTemporaryLove = false;
        hold10sReady = false;
        hold10sPrompted = false;
        isrDownTime = 0;

        currentMode = (SystemMode)(((int)currentMode + 1) % 4);
        isVideoPlaying = false;

        const char* modeNames[] = { "CYBERPET 🐱", "ROBOT EYES 🤖", "CYBER HUD ⚡", "MATRIX RAIN 📟" };
        const uint16_t modeColors[] = { 0xFFE0, 0x07FF, 0x07FF, 0x07E0 };
        triggerTouchVisual(modeNames[(int)currentMode], modeColors[(int)currentMode], 1400, "TOUCH:MODE");

        if (bleConnected && pCharMode) {
            char mChar[2] = { (char)('0' + (int)currentMode), '\0' };
            pCharMode->setValue(std::string(mChar));
            pCharMode->notify();
        }
        Serial.printf("[TOUCH] 5-Tap Gesture! -> Switched System Mode: %d (%s)\n", (int)currentMode, modeNames[(int)currentMode]);
        return;
    }

    // ---------------------------------------------------------
    // 4. MULTI-TAP / DOUBLE-TAP SUB-OPTION CYCLING 🔄
    // (User tapped 2-4 times, finger is UP, and 280ms elapsed without further taps)
    // ---------------------------------------------------------
    if (isrTapCount >= 2 && isrTapCount < 5 && !isDown && (now - isrLastTapEndTime > 280)) {
        isrTapCount = 0;
        lastActivityTime = now;
        isTemporaryLove = false;
        hold10sReady = false;
        hold10sPrompted = false;
        isrDownTime = 0;

        if (currentMode == MODE_CYBERPET) {
            int next = ((int)memePet.currentEmotion + 1) % 7;
            memePet.setEmotion((MemeEmotion)next);
            memePet.defaultEmotion = (MemeEmotion)next;
            const char* emoNames[] = { "LUFFY ⚡", "SHY LOVE 👉👈", "GIGGLE CAT 😸", "SAD BANANA 🍌", "UMARU CRY 😭", "ANGRY CAT 😾", "BUNNY 🐰" };
            triggerTouchVisual(emoNames[next], 0xFFE0, 1000, "TOUCH:DOUBLE");

            if (bleConnected && pCharPet) {
                char emoChar[2] = { (char)('0' + (int)next), '\0' };
                pCharPet->setValue(std::string(emoChar));
                pCharPet->notify();
            }
            Serial.printf("[TOUCH] Double-Tap! -> Mascot: %d (%s)\n", next, emoNames[next]);
        } else if (currentMode == MODE_ROBOT_EYES) {
            int next = robotEyes.cycleStyle();
            const char* styleNames[] = { "CYAN NORMAL 👀", "HAPPY LOVE ❤️", "ANGRY RED 😾", "CYBER GOLD 🟡" };
            const uint16_t styleCols[] = { 0x07FF, 0xF81F, 0xF800, 0xFFE0 };
            triggerTouchVisual(styleNames[next], styleCols[next], 1000, "ROBOT:STYLE");
            if (bleConnected && pCharSet) {
                pCharSet->setValue("ROBOT_MOOD:" + std::to_string(next));
                pCharSet->notify();
            }
            Serial.printf("[TOUCH] Double-Tap! -> Robot Style: %d (%s)\n", next, styleNames[next]);
        } else if (currentMode == MODE_CYBER_HUD) {
            int next = cyberHUD.cycleLayout();
            const char* hudNames[] = { "FULL CYBER HUD ⚡", "BIG CLOCK & DATE ⏰", "MINIMAL DASH 📟" };
            triggerTouchVisual(hudNames[next], 0x07FF, 1000, "HUD:LAYOUT");
            if (bleConnected && pCharSet) {
                pCharSet->setValue("HUD_LAY:" + std::to_string(next));
                pCharSet->notify();
            }
            Serial.printf("[TOUCH] Double-Tap! -> HUD Layout: %d (%s)\n", next, hudNames[next]);
        } else if (currentMode == MODE_MATRIX_RAIN) {
            int next = matrixRain.cycleTheme();
            const char* thmNames[] = { "NEO GREEN 🟢", "CYBER CYAN 🔵", "SYNTH MAGENTA 🟣", "FIRE AMBER 🟡" };
            const uint16_t thmCols[] = { 0x07E0, 0x07FF, 0xF81F, 0xFD20 };
            triggerTouchVisual(thmNames[next], thmCols[next], 1000, "MATRIX:THEME");
            if (bleConnected && pCharSet) {
                pCharSet->setValue("MATRIX_THM:" + std::to_string(next));
                pCharSet->notify();
            }
            Serial.printf("[TOUCH] Double-Tap! -> Matrix Theme: %d (%s)\n", next, thmNames[next]);
        }
        return;
    }

    // ---------------------------------------------------------
    // 5. CONTINUOUS DIRECT HOLD (Finger is DOWN)
    // ---------------------------------------------------------
    if (isDown && isrTapCount == 0 && isrDownTime > 0) {
        uint32_t holdDuration = now - isrDownTime;

        // --- 14+ SECONDS: STUCK SENSOR / TABLE DETECTION -> CANCEL WITH LONG BLINK (450ms) ---
        if (holdDuration >= 14000) {
            hold14sCancelled = true;
            touchStuckLockout = true;
            hold10sReady = false;
            hold10sPrompted = false;
            shyLoveTriggered = false;
            isrDownTime = 0;
            isrTapCount = 0;

            // Distinct long 450ms blue LED blink showing BLE did NOT turn on
            blinkDebugLed(1, 450);
            triggerTouchVisual("HELD >14s: CANCELLED 🛑", 0xF800, 2000, "TOUCH:CANCEL");
            Serial.println("[TOUCH] Held >14s! Action CANCELLED with long blink. Locked out until released.");
            return;
        }

        // --- 10.0s – 14.0s: PROMPT WINDOW (Short 60ms Blue Blink to cue user to release) ---
        if (holdDuration >= 10000 && !hold10sPrompted) {
            hold10sPrompted = true;
            hold10sReady = true;
            lastActivityTime = now;

            // Crisp short 60ms pulse to indicate 10s reached
            blinkDebugLed(1, 60);

            if (!bleActive && !bleConnected) {
                triggerTouchVisual("RELEASE FOR BLE ⚡", 0x07FF, 3500, "TOUCH:READY");
                Serial.println("[TOUCH] 10s Hold reached! Short blue blink. Release finger now to turn BLE ON!");
            } else {
                triggerTouchVisual("RELEASE TO SLEEP 🌙", 0x8410, 3500, "TOUCH:READY");
                Serial.println("[TOUCH] 10s Hold reached! Short blue blink. Release finger now to enter Deep Sleep!");
            }
            return;
        }

        // --- 2.0s – 9.5s: MODE-SPECIFIC SHY / SECRET REACTION ❤️ ---
        if (holdDuration >= 2000 && holdDuration < 9500 && !shyLoveTriggered && !hold10sReady) {
            shyLoveTriggered = true;
            lastActivityTime = now;

            if (currentMode == MODE_CYBER_HUD) {
                cyberHUD.triggerShy(5000);
                triggerTouchVisual("SECRET MSG 💌", 0xF81F, 5000, "TOUCH:SHY:HUD");
                Serial.printf("[TOUCH] 2.0s Hold -> CyberHUD Shy Overlay (%s)\n", cyberHUD.customShyText.c_str());
            } else if (currentMode == MODE_MATRIX_RAIN) {
                matrixRain.triggerHeartRain(5000);
                triggerTouchVisual("HEART RAIN 💖", 0xF81F, 5000, "TOUCH:SHY:MATRIX");
                Serial.printf("[TOUCH] 2.0s Hold -> Matrix Cyber Heart Rain! (%s)\n", matrixRain.customShyText.c_str());
            } else if (currentMode == MODE_ROBOT_EYES) {
                robotEyes.triggerShyLove(5000);
                triggerTouchVisual("HEART EYES ❤️", 0xF81F, 5000, "TOUCH:SHY:ROBOT");
                if (bleConnected && pCharSet) {
                    pCharSet->setValue(std::string("ROBOT_MOOD:1"));
                    pCharSet->notify();
                }
                Serial.printf("[TOUCH] 2.0s Hold -> Robot Eyes Happy Love! (%s)\n", robotEyes.customShyText.c_str());
            } else {
                // MODE_CYBERPET (or default)
                preHoldEmotion = memePet.currentEmotion;
                isTemporaryLove = true;
                loveStartTime = now;

                memePet.setEmotion(EMOTION_SHY);
                currentMode = MODE_CYBERPET;
                isVideoPlaying = false;
                triggerTouchVisual("SHY LOVE ❤️", 0xF81F, 5000, "TOUCH:SHY:PET");

                if (bleConnected && pCharPet) {
                    char emoChar[2] = { (char)('0' + (int)EMOTION_SHY), '\0' };
                    pCharPet->setValue(std::string(emoChar));
                    pCharPet->notify();
                }
                Serial.printf("[TOUCH] 2.0s Hold -> Pet Shy Love! (Reverting to %d in 5s)\n", (int)preHoldEmotion);
            }
            return;
        }
    }

    // ---------------------------------------------------------
    // 6. FINGER RELEASE & HUMAN CONFIRMATION (Finger is UP / !isDown)
    // ---------------------------------------------------------
    if (!isDown) {
        // If user released between 10.0s and 14.0s -> EXECUTE ACTION!
        if (hold10sReady && !hold14sCancelled) {
            hold10sReady = false;
            hold10sPrompted = false;
            shyLoveTriggered = false;
            isrTapCount = 0;
            isrDownTime = 0;
            lastActivityTime = now;

            if (!bleActive && !bleConnected) {
                Serial.println("[TOUCH] Human Release Confirmed (10s-14s) -> Turning BLE ON ⚡");
                startBLE(true);
            } else {
                Serial.println("[TOUCH] Human Release Confirmed (10s-14s) -> Entering Deep Sleep 🌙");
                triggerTouchVisual("POWER OFF 🌙", 0x8410, 1000, "SYS:SLEEP");
                enterDeepSleep();
            }
            return;
        }
    }

    // ---------------------------------------------------------
    // 7. SINGLE TAP TIMEOUT (Poke / Interact 👆)
    // 1 tap registered, finger lifted, and 350ms elapsed without a second tap
    // ---------------------------------------------------------
    if (isrTapCount == 1 && !isDown && (now - isrLastTapEndTime > 350)) {
        isrTapCount = 0;
        lastActivityTime = now;
        if (!shyLoveTriggered && !hold10sReady && !hold14sCancelled) {
            if (currentMode == MODE_CYBERPET) {
                memePet.triggerTap();
                triggerTouchVisual("POKE 👆", 0x07FF, 700, "TOUCH:POKE");
            } else if (currentMode == MODE_ROBOT_EYES) {
                triggerTouchVisual("GLANCE 👀", 0x07FF, 700, "TOUCH:POKE");
            } else if (currentMode == MODE_CYBER_HUD) {
                triggerTouchVisual("TICK ⏱️", 0x07E0, 700, "TOUCH:POKE");
            } else if (currentMode == MODE_MATRIX_RAIN) {
                triggerTouchVisual("GLITCH ⚡", 0x07E0, 700, "TOUCH:POKE");
            } else {
                triggerTouchVisual("TAP 👆", 0x07FF, 700, "TOUCH:POKE");
            }
            Serial.println("[TOUCH] Single Tap Confirmed -> Interact 👆");
        }
    }

    // =========================================================================
    // 8. AUTO-REVERT FROM SHY LOVE AFTER 5 SECONDS BACK TO ORIGINAL STATE
    // =========================================================================
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

    // Notify BLE when Robot Eyes reverts from Shy Love
    static bool lastRobotShyActive = false;
    if (lastRobotShyActive && !robotEyes.isShyLoveActive) {
        if (bleConnected && pCharSet) {
            pCharSet->setValue("ROBOT_MOOD:" + std::to_string(robotEyes.currentStyle));
            pCharSet->notify();
            pCharSet->setValue(std::string("TOUCH:REVERT"));
            pCharSet->notify();
        }
        Serial.printf("[TOUCH] Robot Eyes reverted to mood %d\n", robotEyes.currentStyle);
    }
    lastRobotShyActive = robotEyes.isShyLoveActive;

    // Notify BLE when CyberHUD reverts from Shy Overlay
    static bool lastHudShyActive = false;
    if (lastHudShyActive && !cyberHUD.isShyActive) {
        if (bleConnected && pCharSet) {
            pCharSet->setValue(std::string("TOUCH:REVERT"));
            pCharSet->notify();
        }
        Serial.println("[TOUCH] CyberHUD Shy Overlay closed.");
    }
    lastHudShyActive = cyberHUD.isShyActive;

    // Notify BLE when Matrix Rain reverts from Heart Rain
    static bool lastMatrixShyActive = false;
    if (lastMatrixShyActive && !matrixRain.isHeartRainActive) {
        if (bleConnected && pCharSet) {
            pCharSet->setValue(std::string("TOUCH:REVERT"));
            pCharSet->notify();
        }
        Serial.println("[TOUCH] Matrix Heart Rain ended.");
    }
    lastMatrixShyActive = matrixRain.isHeartRainActive;
}

void enterDeepSleep() {
    Serial.println("[POWER] Entering Deep Sleep...");

    saveBatteryDischargeLog();

    if (bleActive || bleConnected) {
        stopBLE(false);
    }

    // Distinct longer 200ms single flash to signal power-down before sleeping
    blinkDebugLed(1, 200);

    // Wait until touch sensor is completely released before sleeping!
    // Require pin to be continuously LOW for at least 250ms
    uint32_t lowStart = 0;
    uint32_t waitRelease = millis();
    while (millis() - waitRelease < 3000) {
        if (digitalRead(PIN_TOUCH) == LOW) {
            if (lowStart == 0) lowStart = millis();
            if (millis() - lowStart >= 250) break; // Cleanly LOW for 250ms
        } else {
            lowStart = 0;
        }
        delay(15);
    }
    delay(50); // Extra settling delay

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
    // 0. Solid Human Touch Wakeup Verification
    esp_sleep_wakeup_cause_t wakeupReason = esp_sleep_get_wakeup_cause();
    if (wakeupReason == ESP_SLEEP_WAKEUP_GPIO) {
        pinMode(PIN_TOUCH, INPUT_PULLDOWN);
        // Human touch verification: must be stable continuous HIGH for at least 75ms
        uint32_t checkStart = millis();
        bool isHumanTouch = true;
        while (millis() - checkStart < 75) {
            if (digitalRead(PIN_TOUCH) == LOW) {
                isHumanTouch = false;
                break;
            }
            delay(5);
        }
        if (!isHumanTouch) {
            // Stray table noise / electrostatic glitch: Return to deep sleep immediately in <0.08s!
            esp_deep_sleep_enable_gpio_wakeup(1ULL << PIN_TOUCH, ESP_GPIO_WAKEUP_GPIO_HIGH);
            esp_deep_sleep_start();
        }
    }

    Serial.begin(115200);
    delay(200);
    Serial.println("\n=== DIGI KEYCHAIN ENGINE v4.3 STARTUP ===");

    // 1. Release Deep Sleep GPIO Hold
    gpio_hold_dis((gpio_num_t)PIN_TFT_BL);
    pinMode(PIN_TFT_BL, OUTPUT);
    digitalWrite(PIN_TFT_BL, HIGH);

    pinMode(PIN_TOUCH, INPUT_PULLDOWN);
    attachInterrupt(digitalPinToInterrupt(PIN_TOUCH), touchISR, CHANGE);
    if (digitalRead(PIN_TOUCH) == HIGH) {
        touchStuckLockout = true;
        hold14sCancelled = true;
        isrTouchDown = false;
        isrDownTime = 0;
        Serial.println("[TOUCH] Pin HIGH at startup (Surface/USB contact). Locked out until released.");
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
    cyberHUD.setLayout(prefs.getUChar("hud_lay", 0));
    cyberHUD.showSeconds = prefs.getBool("hud_sec", true);
    cyberHUD.showBattery = prefs.getBool("hud_bat", true);
    cyberHUD.showDate = prefs.getBool("hud_date", true);
    cyberHUD.showWaveform = prefs.getBool("hud_wave", true);
    cyberHUD.showCustomText = prefs.getBool("hud_text", true);
    cyberHUD.setCustomText(prefs.getString("hud_msg", "DIGI_HUD // SYS_ONLINE"));
    String shyMsg = prefs.getString("hud_shy", "I LOVE YOU :heart: :sparkles:");
    cyberHUD.setShyText(shyMsg);
    robotEyes.setShyText(shyMsg);
    matrixRain.setShyText(shyMsg);
    robotEyes.setStyle(prefs.getUChar("robot_mood", 0));
    matrixRain.setTheme(prefs.getUChar("matrix_thm", 0));
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
