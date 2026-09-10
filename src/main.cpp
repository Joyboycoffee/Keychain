#include <Arduino.h>
#include <NimBLEDevice.h>
#include "config.h"

static bool bleConnected = false;
static NimBLECharacteristic* pCharMode = nullptr;
static NimBLECharacteristic* pCharBattery = nullptr;
static NimBLECharacteristic* pCharSettings = nullptr;

class ServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer) {
        bleConnected = true;
        Serial.printf("\n*** [BLE] CLIENT CONNECTED! Free Heap: %u bytes ***\n", (unsigned int)ESP.getFreeHeap());
    }
    void onDisconnect(NimBLEServer* pServer) {
        bleConnected = false;
        Serial.printf("\n*** [BLE] CLIENT DISCONNECTED! Free Heap: %u bytes ***\n", (unsigned int)ESP.getFreeHeap());
        NimBLEDevice::startAdvertising();
    }
    void onMTUChange(uint16_t MTU, ble_gap_conn_desc* desc) {
        Serial.printf("[BLE] MTU updated to: %u\n", MTU);
    }
};

class ModeCallback : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pChar) {
        std::string val = pChar->getValue();
        Serial.printf("[BLE] Mode Write received: %s\n", val.c_str());
    }
};

class SettingsCallback : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pChar) {
        std::string val = pChar->getValue();
        Serial.printf("[BLE] Settings Write received: %s\n", val.c_str());
    }
};

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n============================================");
    Serial.println("   DIGI KEYCHAIN - MINIMAL BLE TEST v1.0    ");
    Serial.printf ("   Initial Free Heap: %u bytes\n", (unsigned int)ESP.getFreeHeap());
    Serial.println("============================================");

    // Initialize NimBLE
    NimBLEDevice::init("DIGI_KEYCHAIN");
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);
    NimBLEDevice::setSecurityAuth(false, false, false);

    NimBLEServer* pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());

    NimBLEService* pService = pServer->createService(SERVICE_UUID);

    pCharMode = pService->createCharacteristic(
        CHAR_MODE_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY
    );
    pCharMode->setCallbacks(new ModeCallback());

    pCharBattery = pService->createCharacteristic(
        CHAR_BATTERY_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
    );

    pCharSettings = pService->createCharacteristic(
        CHAR_SETTINGS_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY
    );
    pCharSettings->setCallbacks(new SettingsCallback());

    pService->start();

    // Advertising
    NimBLEAdvertising* pAdv = NimBLEDevice::getAdvertising();
    pAdv->setScanResponse(true);
    pAdv->addServiceUUID(SERVICE_UUID);
    pAdv->start();

    Serial.println("[BLE] Advertising started successfully as DIGI_KEYCHAIN");
    Serial.printf("[BLE] Ready for pairing. Free Heap: %u bytes\n", (unsigned int)ESP.getFreeHeap());
}

void loop() {
    static uint32_t lastPrint = 0;
    if (millis() - lastPrint > 2000) {
        lastPrint = millis();
        Serial.printf("[HEARTBEAT] BLE: %s | Free Heap: %u bytes\n", bleConnected ? "CONNECTED" : "ADVERTISING", (unsigned int)ESP.getFreeHeap());
        
        if (bleConnected && pCharBattery) {
            char bMsg[24];
            snprintf(bMsg, sizeof(bMsg), "4.15|100|0");
            pCharBattery->setValue(bMsg);
            pCharBattery->notify();
        }
    }
    delay(100);
}
