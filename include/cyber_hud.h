#pragma once
#include <LovyanGFX.hpp>
#include "display_setup.h"
#include "config.h"

class CyberHUD {
public:
    int hours = 21;
    int minutes = 35;
    int seconds = 40;
    float batteryPct = 88.0f;
    float weatherTemp = 28.0f;
    String weatherDesc = "CLEAR";
    uint32_t lastSecTick = 0;
    int wavePhase = 0;

    void setTime(int h, int m, int s) {
        hours = h; minutes = m; seconds = s;
    }

    void setWeather(float temp, const String& desc) {
        weatherTemp = temp;
        weatherDesc = desc;
    }

    void update() {
        uint32_t now = millis();
        if (now - lastSecTick >= 1000) {
            lastSecTick = now;
            seconds++;
            if (seconds >= 60) { seconds = 0; minutes++; }
            if (minutes >= 60) { minutes = 0; hours = (hours + 1) % 24; }
        }

        wavePhase = (wavePhase + 4) % 360;

        canvas.fillScreen(TFT_BLACK);

        // Cyberpunk Corner Brackets
        drawSciFiFrame();

        // 1. Top Status Header
        canvas.setTextColor(0x07FF, TFT_BLACK);
        canvas.setTextSize(1);
        canvas.drawString("SYS.NET // ONLINE", 24, 18);
        canvas.drawRightString("ESP32-C3", 216, 18);
        canvas.drawFastHLine(20, 30, 200, 0x03EF);

        // 2. Big Digital Clock
        char timeStr[16];
        snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d", hours, minutes, seconds);
        canvas.setTextColor(0x07E0, TFT_BLACK); // Neon Green
        canvas.setTextSize(2);
        canvas.drawCenterString(timeStr, 120, 48);

        // 3. Telemetry Middle Grid (Temp + Battery)
        canvas.drawRoundRect(20, 80, 95, 50, 6, 0x18C3);
        canvas.drawRoundRect(125, 80, 95, 50, 6, 0x18C3);

        // Temp box
        canvas.setTextColor(0xFDA0, TFT_BLACK); // Amber
        canvas.setTextSize(1);
        canvas.drawString("TEMP", 30, 88);
        char tempStr[16];
        snprintf(tempStr, sizeof(tempStr), "%.1f C", weatherTemp);
        canvas.setTextSize(2);
        canvas.drawString(tempStr, 30, 104);

        // Battery box
        canvas.setTextColor(0x07FF, TFT_BLACK);
        canvas.setTextSize(1);
        canvas.drawString("BATTERY", 135, 88);
        char batStr[16];
        snprintf(batStr, sizeof(batStr), "%d%%", (int)batteryPct);
        canvas.setTextSize(2);
        canvas.drawString(batStr, 135, 104);

        // 4. Animated Sine-Wave Audio / Signal Monitor
        canvas.drawFastHLine(20, 142, 200, 0x10A2);
        for (int x = 20; x < 220; x += 2) {
            float y1 = 175 + sin((x + wavePhase) * 0.08f) * 16.0f;
            float y2 = 175 + sin((x - wavePhase * 1.5f) * 0.05f) * 10.0f;
            canvas.drawPixel(x, (int)y1, 0x07E0);
            canvas.drawPixel(x, (int)y2, 0x07FF);
        }

        // 5. Battery Gauge Bar at Bottom
        canvas.drawRoundRect(20, 206, 200, 14, 4, 0x07FF);
        int barW = (int)((batteryPct / 100.0f) * 192.0f);
        canvas.fillRoundRect(24, 209, barW, 8, 2, (batteryPct > 20) ? 0x07E0 : 0xF800);
    }

private:
    void drawSciFiFrame() {
        uint16_t col = 0x07FF;
        int len = 14;
        // Top-Left
        canvas.drawFastHLine(10, 10, len, col);
        canvas.drawFastVLine(10, 10, len, col);
        // Top-Right
        canvas.drawFastHLine(230 - len, 10, len, col);
        canvas.drawFastVLine(230, 10, len, col);
        // Bottom-Left
        canvas.drawFastHLine(10, 230, len, col);
        canvas.drawFastVLine(10, 230 - len, len, col);
        // Bottom-Right
        canvas.drawFastHLine(230 - len, 230, len, col);
        canvas.drawFastVLine(230, 230 - len, len, col);
    }
};
