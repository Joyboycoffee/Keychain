#pragma once
#include <LovyanGFX.hpp>
#include "display_setup.h"
#include "config.h"
#include "emoji_renderer.h"

class CyberHUD {
public:
    int currentLayout = 0; // 0 = Full HUD, 1 = Big Clock & Date, 2 = Minimalist Dashboard
    bool showSeconds = true;
    bool showBattery = true;
    bool showDate = true;
    bool showWaveform = true;
    bool showCustomText = true;
    String customMessage = "DIGI_HUD // SYS_ONLINE";
    String customShyText = "I LOVE YOU :heart: :sparkles:";
    String dateString = "SUN 13 SEP 2026";

    bool isShyActive = false;
    uint32_t shyStartTime = 0;

    int hours = 13;
    int minutes = 37;
    int seconds = 0;
    float temperature = 27.5;
    String statusText = "ONLINE";
    uint32_t lastTick = 0;
    int animPhase = 0;

    float batteryVoltage = 4.12f;
    int batteryPercent = 95;
    bool isUsbPowered = false;

    void setTime(int h, int m, int s) {
        hours = h; minutes = m; seconds = s;
    }

    void setDate(const String& d) {
        if (d.length() > 0) dateString = d;
    }

    void setWeather(float temp, String stat) {
        temperature = temp; statusText = stat;
    }

    void setBattery(float v, int pct, bool usb) {
        batteryVoltage = v;
        batteryPercent = pct;
        isUsbPowered = usb;
    }

    void setLayout(int lay) {
        currentLayout = (lay >= 0 && lay < 3) ? lay : 0;
    }

    int cycleLayout() {
        currentLayout = (currentLayout + 1) % 3;
        return currentLayout;
    }

    void setCustomText(const String& text) {
        if (text.length() > 0) customMessage = text;
    }

    void setShyText(const String& text) {
        if (text.length() > 0) customShyText = text;
    }

    void triggerShy(uint32_t durMs = 5000) {
        isShyActive = true;
        shyStartTime = millis();
    }

    void setFlags(bool sec, bool bat, bool date, bool wave, bool text) {
        showSeconds = sec;
        showBattery = bat;
        showDate = date;
        showWaveform = wave;
        showCustomText = text;
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

        // Auto revert shy mode after 5 seconds
        if (isShyActive && (millis() - shyStartTime >= 5000)) {
            isShyActive = false;
        }

        canvas.fillScreen(TFT_BLACK);

        if (currentLayout == 0) {
            renderFullHUD();
        } else if (currentLayout == 1) {
            renderBigClockDate();
        } else {
            renderMinimalDashboard();
        }

        // Overlay 2-Second Hold Shy / Love Reaction
        if (isShyActive) {
            renderShyOverlay();
        }
    }

private:
    // =========================================================================
    // LAYOUT 0: FULL CYBER HUD
    // =========================================================================
    void renderFullHUD() {
        // Outer Cyber Frame
        canvas.drawRoundRect(2, 2, 236, 236, 6, 0x07FF);
        canvas.drawRoundRect(4, 4, 232, 232, 4, 0x0210);

        // Header Title / Custom Banner
        canvas.setTextColor(0x07FF, TFT_BLACK);
        canvas.setTextSize(1);
        String header = (showCustomText && customMessage.length() > 0) ? customMessage : "DIGI_HUD // SYS_ONLINE";
        if (header.length() > 20) header = header.substring(0, 20);
        canvas.drawString(header.c_str(), 12, 10);

        // Battery Status Badge
        if (showBattery) {
            drawBatteryBadge(160, 8);
        }

        // Clock Box
        canvas.fillRoundRect(12, 30, 216, 80, 6, 0x0841);
        canvas.drawRoundRect(12, 30, 216, 80, 6, 0x07FF);

        char timeStr[12];
        if (showSeconds) {
            snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d", hours, minutes, seconds);
            canvas.setTextColor(0x07E0, 0x0841);
            canvas.setTextSize(4);
            canvas.drawCenterString(timeStr, 120, (showDate ? 42 : 48));
        } else {
            snprintf(timeStr, sizeof(timeStr), "%02d:%02d", hours, minutes);
            canvas.setTextColor(0x07E0, 0x0841);
            canvas.setTextSize(5);
            canvas.drawCenterString(timeStr, 120, (showDate ? 38 : 46));
        }

        if (showDate) {
            canvas.setTextColor(0x07FF, 0x0841);
            canvas.setTextSize(1);
            canvas.drawCenterString(dateString.c_str(), 120, 88);
        }

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
        if (showWaveform) {
            int baseLineY = 205;
            for (int x = 14; x < 226; x += 4) {
                float angle = (x * 4 + animPhase * 6) * 0.05f;
                int h = (int)(sin(angle) * 12);
                canvas.drawFastVLine(x, baseLineY - abs(h), abs(h) * 2 + 1, 0x07FF);
            }
        } else {
            canvas.drawFastHLine(14, 205, 212, 0x0210);
            canvas.setTextColor(0x8410, TFT_BLACK);
            canvas.setTextSize(1);
            canvas.drawCenterString("DIGI KEYCHAIN // ACTIVE", 120, 214);
        }
    }

    // =========================================================================
    // LAYOUT 1: BIG BOLD CLOCK & DATE
    // =========================================================================
    void renderBigClockDate() {
        // Sleek Minimal Outer Frame
        canvas.drawRoundRect(4, 4, 232, 232, 8, 0x07FF);
        canvas.drawRoundRect(6, 6, 228, 228, 6, 0x0210);

        // Top Header
        if (showCustomText && customMessage.length() > 0) {
            canvas.setTextColor(0x07FF, TFT_BLACK);
            canvas.setTextSize(1);
            String header = customMessage;
            if (header.length() > 22) header = header.substring(0, 22);
            canvas.drawString(header.c_str(), 14, 12);
        }

        // Top Right Battery
        if (showBattery) {
            drawBatteryBadge(160, 10);
        }

        // Big Clock in Center
        char timeStr[12];
        if (showSeconds) {
            snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d", hours, minutes, seconds);
            canvas.setTextColor(0x07E0, TFT_BLACK);
            canvas.setTextSize(4);
            canvas.drawCenterString(timeStr, 120, 68);
        } else {
            snprintf(timeStr, sizeof(timeStr), "%02d:%02d", hours, minutes);
            canvas.setTextColor(0x07E0, TFT_BLACK);
            canvas.setTextSize(6);
            canvas.drawCenterString(timeStr, 120, 62);
        }

        // Divider
        canvas.drawFastHLine(24, 126, 192, 0x07FF);
        canvas.drawFastHLine(36, 128, 168, 0x03EF);

        // Large Date Display
        if (showDate) {
            canvas.setTextColor(0x07FF, TFT_BLACK);
            canvas.setTextSize(2);
            canvas.drawCenterString(dateString.c_str(), 120, 142);
        }

        // Bottom Telemetry Bar
        canvas.fillRoundRect(20, 180, 200, 38, 6, 0x0841);
        canvas.drawRoundRect(20, 180, 200, 38, 6, 0x07E0);

        char bottomInfo[32];
        snprintf(bottomInfo, sizeof(bottomInfo), "%.1f C  |  %s", temperature, statusText.c_str());
        canvas.setTextColor(0x07E0, 0x0841);
        canvas.setTextSize(1);
        canvas.drawCenterString("LIVE TELEMETRY", 120, 186);
        canvas.setTextColor(0xFFFF, 0x0841);
        canvas.setTextSize(2);
        canvas.drawCenterString(bottomInfo, 120, 198);
    }

    // =========================================================================
    // LAYOUT 2: MINIMALIST DASHBOARD
    // =========================================================================
    void renderMinimalDashboard() {
        // Frame
        canvas.drawRoundRect(4, 4, 232, 232, 8, 0xFD20);

        // Top Custom Banner Box
        canvas.fillRoundRect(10, 10, 220, 32, 4, 0x0841);
        canvas.drawRoundRect(10, 10, 220, 32, 4, 0xFD20);
        String header = (showCustomText && customMessage.length() > 0) ? customMessage : "CYBER DASHBOARD";
        if (header.length() > 22) header = header.substring(0, 22);
        canvas.setTextColor(0xFD20, 0x0841);
        canvas.setTextSize(2);
        canvas.drawCenterString(header.c_str(), 120, 18);

        // Main Clock Box
        canvas.fillRoundRect(10, 48, 220, 68, 6, 0x0841);
        canvas.drawRoundRect(10, 48, 220, 68, 6, 0x07FF);

        char timeStr[12];
        if (showSeconds) {
            snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d", hours, minutes, seconds);
            canvas.setTextColor(0x07FF, 0x0841);
            canvas.setTextSize(4);
            canvas.drawCenterString(timeStr, 120, 56);
        } else {
            snprintf(timeStr, sizeof(timeStr), "%02d:%02d", hours, minutes);
            canvas.setTextColor(0x07FF, 0x0841);
            canvas.setTextSize(5);
            canvas.drawCenterString(timeStr, 120, 54);
        }

        if (showDate) {
            canvas.setTextColor(0x07E0, 0x0841);
            canvas.setTextSize(1);
            canvas.drawCenterString(dateString.c_str(), 120, 98);
        }

        // 3-Column Mini Telemetry Cards
        // Card 1: Temp
        canvas.fillRoundRect(10, 122, 69, 52, 4, 0x0841);
        canvas.drawRoundRect(10, 122, 69, 52, 4, 0x07E0);
        canvas.setTextColor(0x07E0, 0x0841);
        canvas.setTextSize(1);
        canvas.drawCenterString("TEMP", 44, 128);
        char tempStr[10];
        snprintf(tempStr, sizeof(tempStr), "%.1f C", temperature);
        canvas.setTextColor(0xFFFF, 0x0841);
        canvas.drawCenterString(tempStr, 44, 148);

        // Card 2: Battery
        canvas.fillRoundRect(85, 122, 69, 52, 4, 0x0841);
        canvas.drawRoundRect(85, 122, 69, 52, 4, 0x07FF);
        canvas.setTextColor(0x07FF, 0x0841);
        canvas.setTextSize(1);
        canvas.drawCenterString("BATTERY", 119, 128);
        char batStr[10];
        if (isUsbPowered) snprintf(batStr, sizeof(batStr), "USB 5V");
        else snprintf(batStr, sizeof(batStr), "%d%%", batteryPercent);
        canvas.setTextColor(0xFFFF, 0x0841);
        canvas.drawCenterString(batStr, 119, 148);

        // Card 3: Status
        canvas.fillRoundRect(160, 122, 70, 52, 4, 0x0841);
        canvas.drawRoundRect(160, 122, 70, 52, 4, 0xF81F);
        canvas.setTextColor(0xF81F, 0x0841);
        canvas.setTextSize(1);
        canvas.drawCenterString("LINK", 195, 128);
        canvas.setTextColor(0xFFFF, 0x0841);
        String st = statusText;
        if (st.length() > 6) st = st.substring(0, 6);
        canvas.drawCenterString(st.c_str(), 195, 148);

        // Waveform / Bottom Accent
        if (showWaveform) {
            int baseLineY = 205;
            for (int x = 12; x < 228; x += 4) {
                float angle = (x * 4 + animPhase * 6) * 0.05f;
                int h = (int)(sin(angle) * 10);
                canvas.drawFastVLine(x, baseLineY - abs(h), abs(h) * 2 + 1, 0xFD20);
            }
        } else {
            canvas.drawFastHLine(12, 205, 216, 0x0841);
            canvas.setTextColor(0x8410, TFT_BLACK);
            canvas.setTextSize(1);
            canvas.drawCenterString("KEYCHAIN COMPACT HUD", 120, 214);
        }
    }

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

    void renderShyOverlay() {
        // Floating Heart Particles
        drawFloatingHearts();

        // Translucent Cyber Love Card
        canvas.fillRoundRect(8, 46, 224, 148, 8, 0x0841);
        canvas.drawRoundRect(8, 46, 224, 148, 8, 0xF81F);
        canvas.drawRoundRect(10, 48, 220, 144, 6, 0x981F);

        // Header Title
        canvas.setTextColor(0xF81F, 0x0841);
        canvas.setTextSize(1);
        canvas.drawCenterString("CYBER LOVE PROTOCOL // 2s HOLD", 120, 58);

        // Render Custom Shy / Secret Message with Emojis
        int textY = 96;
        int txtSize = (customShyText.length() > 14) ? 2 : 3;
        int textW = customShyText.length() * 6 * txtSize;
        int startX = max(16, 120 - (textW / 2));
        EmojiRenderer::renderTextWithEmojis(&canvas, customShyText, startX, textY, txtSize, 0xFFFF, 0x0841);

        // Subtitle / Heart Icon
        canvas.setTextColor(0xFD20, 0x0841);
        canvas.setTextSize(1);
        canvas.drawCenterString("<3 SPECIAL SECRET MESSAGE <3", 120, 142);

        // Progress Timeout Bar
        uint32_t elapsed = millis() - shyStartTime;
        float pct = 1.0f - ((float)elapsed / 5000.0f);
        if (pct < 0.0f) pct = 0.0f;
        if (pct > 1.0f) pct = 1.0f;
        canvas.drawRoundRect(30, 168, 180, 8, 3, 0xF81F);
        canvas.fillRect(32, 170, (int)(176 * pct), 4, 0xF81F);
    }

    void drawFloatingHearts() {
        for (int i = 0; i < 4; i++) {
            int hx = 24 + i * 62 + (int)(sin((animPhase + i * 25) * 0.1f) * 10);
            int hy = 28 + (int)(cos((animPhase + i * 25) * 0.12f) * 14);
            canvas.fillCircle(hx - 3, hy - 2, 4, 0xF81F);
            canvas.fillCircle(hx + 3, hy - 2, 4, 0xF81F);
            canvas.fillTriangle(hx - 7, hy - 1, hx + 7, hy - 1, hx, hy + 7, 0xF81F);
        }
    }
};
