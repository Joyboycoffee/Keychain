#include <Arduino.h>
#include <NimBLEDevice.h>
#include "config.h"
#include "display_setup.h"
#include "robot_eyes.h"
#include "pet_engine.h"
#include "cyber_hud.h"
#include "matrix_rain.h"

// Instantiate Display & Sprite
LGFX_ST7789 tft;
LGFX_Sprite canvas(&tft);

// Modes & Renderers
SystemMode currentMode = MODE_CYBERPET;
RobotEyes robotEyes;
CyberPet cyberPet;
CyberHUD cyberHUD;
MatrixRain matrixRain;

// Scrolling text banner
String customMessage = "CYBERPUNK 2077 // LINK START";
int scrollX = 240;

// Settings & Power
uint8_t screenBrightness = 180; // 0-255 PWM
uint32_t sleepTimeoutMs = 20000; // 20s auto-sleep
uint32_t lastActivityTime = 0;
bool bleConnected = false;

// Touch Gesture Tracker
uint32_t touchStartTime = 0;
uint32_t touchReleaseTime = 0;
int tapCount = 0;
bool isTouching = false;

// =========================================================================
// BLE CALLBACKS
// =========================================================================
class ServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer) {
        bleConnected = true;
        lastActivityTime = millis();
        // Turn ON Blue LED to indicate Bluetooth Connected!
        digitalWrite(PIN_DEBUG_LED, LOW); // Active-low on ESP32-C3 SuperMini
    }
    void onDisconnect(NimBLEServer* pServer) {
        bleConnected = false;
        lastActivityTime = millis();
        // Turn OFF Blue LED
        digitalWrite(PIN_DEBUG_LED, HIGH);
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
                cyberHUD.setWeather(temp, "PHONE_SYNC");
            }
            lastActivityTime = millis();
        }
    }
};

class SettingsCallback : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pChar) {
        std::string val = pChar->getValue();
        if (val.length() > 0) {
            int br = atoi(val.c_str());
            if (br > 0 && br <= 255) {
                screenBrightness = br;
                tft.setBrightness(screenBrightness);
            }
            lastActivityTime = millis();
        }
    }
};

// =========================================================================
// TOUCH GESTURE HANDLER (RUB = HAPPY, SPAM = ANGRY, TAP = CYCLE)
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

    if (isTouching && (now - touchStartTime > 1200)) {
        cyberPet.setMood(MOOD_HAPPY);
        robotEyes.setMood(MOOD_HAPPY);
        lastActivityTime = now;
    }

    if (!isTouching && tapCount > 0 && (now - touchReleaseTime > 400)) {
        if (tapCount >= 3) {
            cyberPet.setMood(MOOD_ANGRY);
            robotEyes.setMood(MOOD_ANGRY);
        } else if (tapCount == 1) {
            if (currentMode == MODE_CYBERPET) {
                cyberPet.setAvatar((PetAvatar)((cyberPet.currentAvatar + 1) % 3));
            } else {
                currentMode = (SystemMode)((currentMode + 1) % 5);
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
    // Turn OFF debug LED before sleeping
    pinMode(PIN_DEBUG_LED, OUTPUT);
    digitalWrite(PIN_DEBUG_LED, HIGH);

    for (int b = screenBrightness; b >= 0; b -= 15) {
        tft.setBrightness(b);
        delay(15);
    }
    tft.writeCommand(0x10);

    gpio_wakeup_enable((gpio_num_t)PIN_TOUCH, GPIO_INTR_HIGH_LEVEL);
    esp_deep_sleep_enable_gpio_wakeup(1ULL << PIN_TOUCH, ESP_GPIO_WAKEUP_GPIO_HIGH);

    esp_deep_sleep_start();
}

// =========================================================================
// SETUP & LOOP
// =========================================================================
void setup() {
    Serial.begin(115200);

    pinMode(PIN_TOUCH, INPUT);
    pinMode(PIN_TFT_BL, OUTPUT);
    
    // Configure Blue Debug LED (GPIO 8)
    pinMode(PIN_DEBUG_LED, OUTPUT);
    // Quick 2-blink startup test
    digitalWrite(PIN_DEBUG_LED, LOW); delay(80);
    digitalWrite(PIN_DEBUG_LED, HIGH); delay(80);
    digitalWrite(PIN_DEBUG_LED, LOW); delay(80);
    digitalWrite(PIN_DEBUG_LED, HIGH);

    tft.init();
    tft.setRotation(0);
    tft.setBrightness(screenBrightness);

    canvas.setColorDepth(16);
    canvas.createSprite(240, 240);

    NimBLEDevice::init("CYBER_KEYCHAIN");
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);
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

    pService->start();

    NimBLEAdvertising* pAdv = NimBLEDevice::getAdvertising();
    pAdv->addServiceUUID(SERVICE_UUID);
    pAdv->start();

    lastActivityTime = millis();
}

void loop() {
    processTouch();

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
            canvas.setTextColor(0x07E0, TFT_BLACK);
            canvas.setTextSize(2);
            canvas.drawString(customMessage, scrollX, 105);
            scrollX -= 3;
            if (scrollX < -((int)customMessage.length() * 16)) scrollX = 240;
            break;
    }

    canvas.pushSprite(0, 0);

    if (!bleConnected && (millis() - lastActivityTime > sleepTimeoutMs)) {
        enterDeepSleep();
    }

    delay(10);
}
