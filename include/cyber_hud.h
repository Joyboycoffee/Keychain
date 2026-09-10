#pragma once
#include <LovyanGFX.hpp>
#include "display_setup.h"
#include "config.h"

class CyberHUD {
private:
    int hours = 13;
    int minutes = 37;
    int seconds = 0;
    float temperature = 27.5;
    String statusText = "ONLINE";
    uint32_t lastTick = 0;
    int animPhase = 0;

public:
    float batteryVoltage = 4.12f;
    int batteryPercent = 95;
    bool isUsbPowered = false;

    void setTime(int h, int m, int s) {
        hours = h; minutes = m; seconds = s;
    }

    void setWeather(float temp, String stat) {
        temperature = temp; statusText = stat;
    }

    void setBattery(float v, int pct, bool usb) {
        batteryVoltage = v;
        batteryPercent = pct;
        isUsbPowered = usb;
    }

    void update() {
        if (millis() - lastTick > 1000) {
            lastTick = millis();
            seconds++;
            if (seconds >= 60) {
                seconds = 0;
                minutes++;
                if (minutes >= 60) {
                    minutes = 0;
                    hours = (hours + 1) % 24;
                }
            }
        }
        animPhase = (animPhase + 1) % 360;

        canvas.fillScreen(TFT_BLACK);

        // Cyber Outer Frame
        canvas.drawRoundRect(2, 2, 236, 236, 6, 0x07FF);
        canvas.drawRoundRect(4, 4, 232, 232, 4, 0x0210);

        // Header Title
        canvas.setTextColor(0x07FF, TFT_BLACK);
        canvas.setTextSize(1);
        canvas.drawString("DIGI_HUD // SYS_ONLINE", 12, 10);

        // Battery Status Badge
        drawBatteryBadge(160, 8);

        // Clock Box
        canvas.fillRoundRect(12, 30, 216, 80, 6, 0x0841);
        canvas.drawRoundRect(12, 30, 216, 80, 6, 0x07FF);

        char timeStr[12];
        snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d", hours, minutes, seconds);
        canvas.setTextColor(0x07E0, 0x0841);
        canvas.setTextSize(4);
        canvas.drawCenterString(timeStr, 120, 48);

        // Sci-Fi Status Data Row
        canvas.fillRoundRect(12, 118, 104, 52, 4, 0x0841);
        canvas.drawRoundRect(12, 118, 104, 52, 4, 0x07E0);
        canvas.setTextColor(0x07E0, 0x0841);
        canvas.setTextSize(1);
        canvas.drawString("TEMP // C", 18, 124);
        char tempStr[10];
        snprintf(tempStr, sizeof(tempStr), "%.1f C", temperature);
        canvas.setTextSize(2);
        canvas.drawString(tempStr, 18, 142);

        canvas.fillRoundRect(124, 118, 104, 52, 4, 0x0841);
        canvas.drawRoundRect(124, 118, 104, 52, 4, 0x07FF);
        canvas.setTextColor(0x07FF, 0x0841);
        canvas.setTextSize(1);
        canvas.drawString("LINK STATUS", 130, 124);
        canvas.setTextSize(2);
        canvas.drawString(statusText, 130, 142);

        // Animated Waveform Footer
        int baseLineY = 205;
        for (int x = 14; x < 226; x += 4) {
            float angle = (x * 4 + animPhase * 6) * 0.05f;
            int h = (int)(sin(angle) * 12);
            canvas.drawFastVLine(x, baseLineY - abs(h), abs(h) * 2 + 1, 0x07FF);
        }
    }

private:
    void drawBatteryBadge(int x, int y) {
        uint16_t col = (batteryPercent > 20) ? 0x07E0 : 0xF800; // Green or Red
        if (isUsbPowered) col = 0x07FF; // Cyan on USB

        canvas.drawRoundRect(x, y, 32, 14, 2, col);
        canvas.fillRect(x + 32, y + 4, 3, 6, col);

        int fillW = map(batteryPercent, 0, 100, 0, 26);
        if (fillW > 0) {
            canvas.fillRect(x + 3, y + 3, fillW, 8, col);
        }

        canvas.setTextColor(col, TFT_BLACK);
        canvas.setTextSize(1);
        char bStr[8];
        snprintf(bStr, sizeof(bStr), "%d%%", batteryPercent);
        canvas.drawString(bStr, x + 38, y + 3);
    }
};
