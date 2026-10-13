#pragma once
#include <LovyanGFX.hpp>
#include "display_setup.h"
#include "config.h"
#include "emoji_renderer.h"

class CyberHUD {
public:
    int currentLayout = 0; // 0 = Full HUD with Eyes, 1 = Big Clock & Date, 2 = Minimalist Dashboard, 3 = Minimalistic One
    bool is12HourFormat = true; // Default 12-hour format with AM/PM
    bool showSeconds = true;
    bool showBattery = true;
    bool showDate = true;
    bool showWaveform = true;
    bool showCustomText = true;
    String customMessage = "SPEARHEAD // HUD";
    String customShyText = "I LOVE YOU <3";
    String dateString = "SUN 13 SEP 2026";
    String fullDateString = "Tuesday, 13th October";

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

    void setFullDate(const String& fd) {
        if (fd.length() > 0) fullDateString = fd;
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
        currentLayout = (lay >= 0 && lay < 4) ? lay : 0;
    }

    void setTimeFormat(bool is12) {
        is12HourFormat = is12;
    }

    int cycleLayout() {
        currentLayout = (currentLayout + 1) % 4;
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

    void tick() {
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
    }

    void update() {
        tick();
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
        } else if (currentLayout == 2) {
            renderMinimalDashboard();
        } else {
            renderMinimalisticClock();
        }

        // Overlay 2-Second Hold Shy / Love Reaction
        if (isShyActive && currentLayout != 3) {
            renderShyOverlay();
        }
    }

private:
    // =========================================================================
    // DRAW ANIMATED CYBER ROBOT EYES ON TOP OF HUD
    // =========================================================================
    void drawCyberEyes(int centerX, int centerY) {
        // Blink logic: smooth blink every 4 seconds
        int blinkCycle = (animPhase * 3) % 300;
        int eyeHeight = 16;
        if (blinkCycle > 280) {
            int blinkProgress = blinkCycle - 280; // 0 to 20
            if (blinkProgress <= 10) {
                eyeHeight = 16 - (blinkProgress * 14 / 10);
            } else {
                eyeHeight = 2 + ((blinkProgress - 10) * 14 / 10);
            }
            if (eyeHeight < 2) eyeHeight = 2;
        }

        // Gaze shift logic (smooth eye pupils looking left / right)
        int gazeOffset = (int)(sin(animPhase * 0.05f) * 4);

        int eyeWidth = 24;
        int eyeSpacing = 18;
        int leftEyeX = centerX - eyeWidth - (eyeSpacing / 2);
        int rightEyeX = centerX + (eyeSpacing / 2);
        int eyeY = centerY - (eyeHeight / 2);

        // Cybernetic outer visor glow
        canvas.drawRoundRect(leftEyeX - 2, eyeY - 2, eyeWidth + 4, eyeHeight + 4, 3, 0x0210);
        canvas.drawRoundRect(rightEyeX - 2, eyeY - 2, eyeWidth + 4, eyeHeight + 4, 3, 0x0210);

        // Vivid Cyan Eyes
        canvas.fillRoundRect(leftEyeX, eyeY, eyeWidth, eyeHeight, 2, 0x07FF);
        canvas.fillRoundRect(rightEyeX, eyeY, eyeWidth, eyeHeight, 2, 0x07FF);

        // Dark pupil slit when eyes are open
        if (eyeHeight >= 8) {
            canvas.fillRect(leftEyeX + 8 + gazeOffset, eyeY + 2, 6, eyeHeight - 4, 0x0000);
            canvas.fillRect(rightEyeX + 8 + gazeOffset, eyeY + 2, 6, eyeHeight - 4, 0x0000);
            
            // Eye highlight spark
            canvas.fillRect(leftEyeX + 4, eyeY + 3, 3, 3, 0xFFFF);
            canvas.fillRect(rightEyeX + 4, eyeY + 3, 3, 3, 0xFFFF);
        }
    }

    // =========================================================================
    // LAYOUT 0: FULL CYBER HUD WITH TOP ROBOT EYES & 12/24HR TIME
    // =========================================================================
    void renderFullHUD() {
        // Outer Cyber Frame
        canvas.drawRoundRect(2, 2, 236, 236, 4, 0x07FF);
        canvas.drawRoundRect(4, 4, 232, 232, 2, 0x0210);

        // Header Title / Custom Banner on top-left
        canvas.setTextColor(0x07FF, TFT_BLACK);
        canvas.setTextSize(1);
        String header = (showCustomText && customMessage.length() > 0) ? customMessage : "SPEARHEAD // HUD";
        if (header.length() > 16) header = header.substring(0, 16);
        canvas.drawString(header.c_str(), 10, 8);

        // Battery Status Badge on top-right
        if (showBattery) {
            drawBatteryBadge(164, 8);
        }

        // 1. TOP ANIMATED CYBER EYES
        drawCyberEyes(120, 36);

        // 2. MAIN DIGITAL CLOCK BOX
        canvas.fillRoundRect(10, 56, 220, 64, 4, 0x0841);
        canvas.drawRoundRect(10, 56, 220, 64, 4, 0x07FF);

        int dispH = is12HourFormat ? (hours % 12 == 0 ? 12 : hours % 12) : hours;
        char timeStr[16];
        if (showSeconds) {
            snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d", dispH, minutes, seconds);
            canvas.setTextColor(0x07E0, 0x0841);
            canvas.setTextSize(4);
            canvas.drawCenterString(timeStr, 114, 66);
        } else {
            snprintf(timeStr, sizeof(timeStr), "%02d:%02d", dispH, minutes);
            canvas.setTextColor(0x07E0, 0x0841);
            canvas.setTextSize(5);
            canvas.drawCenterString(timeStr, 112, 64);
        }

        // AM / PM indicator if 12-hour format
        if (is12HourFormat) {
            canvas.setTextColor(0x07FF, 0x0841);
            canvas.setTextSize(1);
            const char* ampm = (hours >= 12) ? "PM" : "AM";
            canvas.drawString(ampm, 202, 64);
        }

        // Date Display below clock digits
        if (showDate) {
            canvas.setTextColor(0x07FF, 0x0841);
            canvas.setTextSize(1);
            canvas.drawCenterString(dateString.c_str(), 120, 106);
        }

        // 3. SCI-FI STATUS DATA ROW (TEMP & CONNECTION)
        canvas.fillRoundRect(10, 126, 106, 44, 3, 0x0841);
        canvas.drawRoundRect(10, 126, 106, 44, 3, 0x07E0);
        canvas.setTextColor(0x07E0, 0x0841);
        canvas.setTextSize(1);
        canvas.drawString("TEMPERATURE", 16, 131);
        char tempStr[12];
        snprintf(tempStr, sizeof(tempStr), "%.1f C", temperature);
        canvas.setTextSize(2);
        canvas.drawString(tempStr, 16, 146);

        canvas.fillRoundRect(124, 126, 106, 44, 3, 0x0841);
        canvas.drawRoundRect(124, 126, 106, 44, 3, 0x07FF);
        canvas.setTextColor(0x07FF, 0x0841);
        canvas.setTextSize(1);
        canvas.drawString("LINK STATUS", 130, 131);
        canvas.setTextSize(2);
        canvas.drawString(statusText, 130, 146);

        // 4. ANIMATED WAVEFORM FOOTER
        if (showWaveform) {
            int baseLineY = 196;
            for (int x = 12; x < 228; x += 4) {
                float angle = (x * 4 + animPhase * 6) * 0.05f;
                int h = (int)(sin(angle) * 10);
                canvas.drawFastVLine(x, baseLineY - abs(h), abs(h) * 2 + 1, 0x07FF);
            }
        }

        // Bottom Tag
        canvas.setTextColor(0x8410, TFT_BLACK);
        canvas.setTextSize(1);
        canvas.drawCenterString("SPEARHEAD // TACTICAL HUD", 120, 218);
    }

    // =========================================================================
    // LAYOUT 1: BIG CLOCK, EYES & DATE
    // =========================================================================
    void renderBigClockDate() {
        // Frame
        canvas.drawRoundRect(4, 4, 232, 232, 4, 0x07FF);
        canvas.drawRoundRect(6, 6, 228, 228, 2, 0x0210);

        // Top Header
        if (showCustomText && customMessage.length() > 0) {
            canvas.setTextColor(0x07FF, TFT_BLACK);
            canvas.setTextSize(1);
            String header = customMessage;
            if (header.length() > 18) header = header.substring(0, 18);
            canvas.drawString(header.c_str(), 14, 10);
        }

        if (showBattery) {
            drawBatteryBadge(164, 10);
        }

        // Top Cyber Eyes
        drawCyberEyes(120, 36);

        // Big Clock in Center
        int dispH = is12HourFormat ? (hours % 12 == 0 ? 12 : hours % 12) : hours;
        char timeStr[16];
        if (showSeconds) {
            snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d", dispH, minutes, seconds);
            canvas.setTextColor(0x07E0, TFT_BLACK);
            canvas.setTextSize(4);
            canvas.drawCenterString(timeStr, 114, 70);
        } else {
            snprintf(timeStr, sizeof(timeStr), "%02d:%02d", dispH, minutes);
            canvas.setTextColor(0x07E0, TFT_BLACK);
            canvas.setTextSize(5);
            canvas.drawCenterString(timeStr, 112, 66);
        }

        if (is12HourFormat) {
            canvas.setTextColor(0x07FF, TFT_BLACK);
            canvas.setTextSize(1);
            const char* ampm = (hours >= 12) ? "PM" : "AM";
            canvas.drawString(ampm, 202, 66);
        }

        // Divider
        canvas.drawFastHLine(24, 122, 192, 0x07FF);
        canvas.drawFastHLine(36, 124, 168, 0x03EF);

        // Large Date Display
        if (showDate) {
            canvas.setTextColor(0x07FF, TFT_BLACK);
            canvas.setTextSize(2);
            canvas.drawCenterString(dateString.c_str(), 120, 136);
        }

        // Bottom Telemetry Bar
        canvas.fillRoundRect(16, 172, 208, 44, 4, 0x0841);
        canvas.drawRoundRect(16, 172, 208, 44, 4, 0x07E0);

        char bottomInfo[32];
        snprintf(bottomInfo, sizeof(bottomInfo), "%.1f C  |  %s", temperature, statusText.c_str());
        canvas.setTextColor(0x07E0, 0x0841);
        canvas.setTextSize(1);
        canvas.drawCenterString("LIVE TELEMETRY", 120, 178);
        canvas.setTextColor(0xFFFF, 0x0841);
        canvas.setTextSize(2);
        canvas.drawCenterString(bottomInfo, 120, 192);
    }

    // =========================================================================
    // LAYOUT 2: MINIMALIST DASHBOARD
    // =========================================================================
    void renderMinimalDashboard() {
        canvas.drawRoundRect(4, 4, 232, 232, 4, 0xFD20);

        // Top Eyes
        drawCyberEyes(120, 24);

        // Main Clock Box
        canvas.fillRoundRect(10, 44, 220, 68, 4, 0x0841);
        canvas.drawRoundRect(10, 44, 220, 68, 4, 0x07FF);

        int dispH = is12HourFormat ? (hours % 12 == 0 ? 12 : hours % 12) : hours;
        char timeStr[16];
        if (showSeconds) {
            snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d", dispH, minutes, seconds);
            canvas.setTextColor(0x07FF, 0x0841);
            canvas.setTextSize(4);
            canvas.drawCenterString(timeStr, 114, 52);
        } else {
            snprintf(timeStr, sizeof(timeStr), "%02d:%02d", dispH, minutes);
            canvas.setTextColor(0x07FF, 0x0841);
            canvas.setTextSize(5);
            canvas.drawCenterString(timeStr, 112, 50);
        }

        if (is12HourFormat) {
            canvas.setTextColor(0xFD20, 0x0841);
            canvas.setTextSize(1);
            const char* ampm = (hours >= 12) ? "PM" : "AM";
            canvas.drawString(ampm, 202, 50);
        }

        if (showDate) {
            canvas.setTextColor(0x07E0, 0x0841);
            canvas.setTextSize(1);
            canvas.drawCenterString(dateString.c_str(), 120, 94);
        }

        // 3-Column Mini Telemetry Cards
        // Card 1: Temp
        canvas.fillRoundRect(10, 120, 69, 48, 3, 0x0841);
        canvas.drawRoundRect(10, 120, 69, 48, 3, 0x07E0);
        canvas.setTextColor(0x07E0, 0x0841);
        canvas.setTextSize(1);
        canvas.drawCenterString("TEMP", 44, 126);
        char tempStr[10];
        snprintf(tempStr, sizeof(tempStr), "%.1f C", temperature);
        canvas.drawCenterString(tempStr, 44, 142);

        // Card 2: Bat
        canvas.fillRoundRect(85, 120, 70, 48, 3, 0x0841);
        canvas.drawRoundRect(85, 120, 70, 48, 3, 0x07FF);
        canvas.setTextColor(0x07FF, 0x0841);
        canvas.setTextSize(1);
        canvas.drawCenterString("BATTERY", 120, 126);
        char batStr[10];
        if (isUsbPowered) snprintf(batStr, sizeof(batStr), "USB");
        else snprintf(batStr, sizeof(batStr), "%d%%", batteryPercent);
        canvas.drawCenterString(batStr, 120, 142);

        // Card 3: Link
        canvas.fillRoundRect(161, 120, 69, 48, 3, 0x0841);
        canvas.drawRoundRect(161, 120, 69, 48, 3, 0xFD20);
        canvas.setTextColor(0xFD20, 0x0841);
        canvas.setTextSize(1);
        canvas.drawCenterString("LINK", 195, 126);
        canvas.drawCenterString(statusText.c_str(), 195, 142);

        // Bottom Bar
        canvas.setTextColor(0x8410, TFT_BLACK);
        canvas.setTextSize(1);
        canvas.drawCenterString("SPEARHEAD // DASHBOARD", 120, 214);
    }

    // =========================================================================
    // 2-SECOND HOLD SECRET REACTION OVERLAY
    // =========================================================================
    void renderShyOverlay() {
        int cardW = 196;
        int cardH = 36;
        int cardX = (240 - cardW) / 2;
        int cardY = 192;

        canvas.fillRoundRect(cardX, cardY, cardW, cardH, 4, 0x0841);
        canvas.drawRoundRect(cardX, cardY, cardW, cardH, 4, 0xF81F);

        int txtSize = (customShyText.length() > 16) ? 1 : 2;
        int textW = customShyText.length() * 6 * txtSize;
        int startX = max(cardX + 6, 120 - (textW / 2));
        EmojiRenderer::renderTextWithEmojis(&canvas, customShyText, startX, cardY + (txtSize == 1 ? 13 : 10), txtSize, 0xFFFF, 0x0841);
    }

    void drawBatteryBadge(int x, int y) {
        // Consistent Rounded Battery Indicator (28 x 14 px)
        uint16_t bCol = (batteryPercent <= 20) ? 0xF800 : (batteryPercent <= 50 ? 0xFFE0 : 0x07E0);
        if (isUsbPowered) bCol = 0x07E0;

        // Clean white shell
        canvas.drawRoundRect(x, y, 28, 14, 3, 0xFFFF);
        // Terminal tip (+) on right
        canvas.fillRect(x + 28, y + 4, 3, 6, 0xFFFF);

        // Filled level inside shell
        int fillW = (batteryPercent * 22) / 100;
        if (fillW < 2 && batteryPercent > 0) fillW = 2;
        if (fillW > 22) fillW = 22;

        if (isUsbPowered) {
            // Charging fill: full green bar with lightning bolt
            canvas.fillRect(x + 3, y + 3, 22, 8, bCol);
            // 1px dark lightning bolt in center
            canvas.drawLine(x + 15, y + 3, x + 12, y + 7, 0x0000);
            canvas.drawLine(x + 12, y + 7, x + 16, y + 7, 0x0000);
            canvas.drawLine(x + 16, y + 7, x + 13, y + 10, 0x0000);
        } else {
            canvas.fillRect(x + 3, y + 3, fillW, 8, bCol);
        }
    }

public:
    // =========================================================================
    // LAYOUT 3: MINIMALISTIC ONE (FREDOKA BOLD CLOCK & PURE BLACK BACKGROUND)
    // =========================================================================
    void renderMinimalisticClock() {
        tick();

        // 1. Consistent Battery Badge in top-right corner
        if (showBattery) {
            drawBatteryBadge(196, 14);
        }

        // 2. Big Time in HH:MM format only (no seconds) in rounded bold font
        int dispH = is12HourFormat ? (hours % 12 == 0 ? 12 : hours % 12) : hours;
        char timeStr[16];
        snprintf(timeStr, sizeof(timeStr), "%02d:%02d", dispH, minutes);

        canvas.setFont(&fonts::DejaVu56);
        canvas.setTextDatum(textdatum_t::middle_center);
        canvas.setTextColor(0xFFFF, TFT_BLACK);
        canvas.drawString(timeStr, 120, 104);

        // 3. Current Date underneath in clean rounded font: e.g. "Tuesday, 13th October"
        canvas.setFont(&fonts::FreeSansBold9pt7b);
        canvas.setTextColor(0xFFFF, TFT_BLACK);
        canvas.drawString(fullDateString.c_str(), 120, 154);

        // Restore default font & datum
        canvas.setFont(&fonts::Font0);
        canvas.setTextDatum(textdatum_t::top_left);

        // Secret Love / Shy Overlay (2s hold)
        if (isShyActive) {
            renderShyOverlay();
        }
    }
};
