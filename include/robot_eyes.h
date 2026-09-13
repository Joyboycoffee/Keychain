#pragma once
#include <LovyanGFX.hpp>
#include "display_setup.h"
#include "config.h"
#include "emoji_renderer.h"

class RobotEyes {
public:
    int currentStyle = 0; // 0 = Cyan Normal, 1 = Happy Love, 2 = Angry Slant, 3 = Cyber Gold Scanner
    bool isShyLoveActive = false;
    uint32_t shyLoveStartTime = 0;
    int preShyStyle = 0;
    String customShyText = "I LOVE YOU :heart: :sparkles:";

private:
    float eyeWidth = 55;
    float eyeHeight = 75;
    float eyeRadius = 18;
    float eyeSpacing = 35;

    float currentX = 120;
    float currentY = 120;
    float targetX = 120;
    float targetY = 120;

    float currentH = 75;
    float targetH = 75;

    uint32_t lastGazeChange = 0;
    uint32_t lastBlinkTime = 0;
    bool isBlinking = false;
    int scanPhase = 0;

    PetMood currentMood = MOOD_IDLE;
    uint16_t eyeColor = 0x07FF; // Default Cyan (RGB565)

public:
    void setStyle(int style) {
        currentStyle = (style >= 0 && style < 4) ? style : 0;
        if (currentStyle == 0) {
            currentMood = MOOD_IDLE;
            eyeColor = 0x07FF; // Cyan
        } else if (currentStyle == 1) {
            currentMood = MOOD_LOVE;
            eyeColor = 0xF81F; // Pink / Magenta
        } else if (currentStyle == 2) {
            currentMood = MOOD_ANGRY;
            eyeColor = 0xF800; // Fierce Red
        } else if (currentStyle == 3) {
            currentMood = MOOD_HAPPY;
            eyeColor = 0xFFE0; // Cyber Gold
        }
    }

    int cycleStyle() {
        currentStyle = (currentStyle + 1) % 4;
        setStyle(currentStyle);
        return currentStyle;
    }

    void setShyText(const String& txt) {
        if (txt.length() > 0) customShyText = txt;
    }

    void triggerShyLove(uint32_t durMs = 5000) {
        if (!isShyLoveActive) preShyStyle = currentStyle;
        isShyLoveActive = true;
        shyLoveStartTime = millis();
        setStyle(1); // Switch to Happy Love
        targetY = 90; // Move eyes slightly up to accommodate love card
    }

    void setMood(PetMood mood) {
        currentMood = mood;
        if (mood == MOOD_HAPPY || mood == MOOD_LOVE) {
            currentStyle = 1;
            eyeColor = 0xF81F; // Pink / Magenta
        } else if (mood == MOOD_ANGRY) {
            currentStyle = 2;
            eyeColor = 0xF800; // Fierce Red
        } else {
            currentStyle = 0;
            eyeColor = 0x07FF; // Glowing Cyan
        }
    }

    void update() {
        uint32_t now = millis();
        scanPhase = (scanPhase + 4) % 360;

        if (isShyLoveActive && (now - shyLoveStartTime >= 5000)) {
            isShyLoveActive = false;
            targetY = 120;
            setStyle(preShyStyle);
        }

        // 1. Random Gaze Tracking (looks around naturally)
        if (now - lastGazeChange > (currentStyle == 3 ? 1400 : 2500) && !isBlinking) {
            lastGazeChange = now;
            targetX = 120 + random(-25, 26);
            targetY = isShyLoveActive ? 90 : (120 + random(-15, 16));
        }

        // 2. Random Periodic Blinking
        if (now - lastBlinkTime > 3800 && !isBlinking) {
            isBlinking = true;
            lastBlinkTime = now;
            targetH = 4; // Flat line blink
        } else if (isBlinking && now - lastBlinkTime > 150) {
            isBlinking = false;
            targetH = eyeHeight;
        }

        // Smooth Interpolation (Easing)
        currentX += (targetX - currentX) * 0.2f;
        currentY += (targetY - currentY) * 0.2f;
        currentH += (targetH - currentH) * 0.35f;

        // Render to canvas sprite
        canvas.fillScreen(TFT_BLACK);

        float leftEyeX = currentX - (eyeWidth / 2) - (eyeSpacing / 2);
        float rightEyeX = currentX + (eyeWidth / 2) + (eyeSpacing / 2);
        float eyeY = currentY - (currentH / 2);

        if (isShyLoveActive || currentStyle == 1) {
            // Style 1 / Shy Love: Upward Curved Eyes + Floating Hearts + Blushing Cheeks
            drawHappyEye(leftEyeX, currentY, eyeWidth, 0xF81F);
            drawHappyEye(rightEyeX, currentY, eyeWidth, 0xF81F);

            // Blushing Pink Cheeks
            canvas.fillRoundRect(22, (int)currentY + 26, 36, 12, 6, 0xF81F);
            canvas.fillRoundRect(182, (int)currentY + 26, 36, 12, 6, 0xF81F);

            // Floating Animated Hearts
            for (int i = 0; i < 4; i++) {
                int hx = 35 + i * 55 + (int)(sin((scanPhase + i * 30) * 0.1f) * 8);
                int hy = 40 + (int)(cos((scanPhase + i * 25) * 0.12f) * 12);
                drawFloatingHeart(hx, hy, 0xF81F);
            }

            // Shy Love Message Overlay Card
            if (isShyLoveActive) {
                renderShyLoveCard();
            }
        } else if (currentStyle == 2) {
            // Style 2: Angry Slant
            drawAngryEye(leftEyeX, currentY, eyeWidth, currentH, true, 0xF800);
            drawAngryEye(rightEyeX, currentY, eyeWidth, currentH, false, 0xF800);
        } else if (currentStyle == 3) {
            // Style 3: Cyber Gold Scanner (Gleaming Gold Eyes + Animated Scanning Beam)
            canvas.fillRoundRect(leftEyeX - (eyeWidth / 2), eyeY, eyeWidth, currentH, eyeRadius, 0xFFE0);
            canvas.fillRoundRect(rightEyeX - (eyeWidth / 2), eyeY, eyeWidth, currentH, eyeRadius, 0xFFE0);

            // Pupil highlights
            if (currentH > 25) {
                canvas.fillCircle(leftEyeX, eyeY + (currentH * 0.35f), 7, TFT_WHITE);
                canvas.fillCircle(rightEyeX, eyeY + (currentH * 0.35f), 7, TFT_WHITE);
                
                // Animated Scanning Laser Bar
                int scanY = eyeY + 4 + (int)((sin(scanPhase * 0.05f) + 1.0f) * 0.5f * (currentH - 12));
                canvas.drawFastHLine(leftEyeX - (eyeWidth / 2) + 2, scanY, eyeWidth - 4, 0xFD20);
                canvas.drawFastHLine(rightEyeX - (eyeWidth / 2) + 2, scanY, eyeWidth - 4, 0xFD20);
            }

            // Outer Target Reticles
            canvas.drawCircle(currentX, currentY, 100, 0x2945);
            canvas.drawFastHLine(currentX - 110, currentY, 20, 0xFFE0);
            canvas.drawFastHLine(currentX + 90, currentY, 20, 0xFFE0);
        } else {
            // Style 0: Normal Expressive Glowing Cyan Eyes with Glossy Highlights
            canvas.fillRoundRect(leftEyeX - (eyeWidth / 2), eyeY, eyeWidth, currentH, eyeRadius, 0x07FF);
            canvas.fillRoundRect(rightEyeX - (eyeWidth / 2), eyeY, eyeWidth, currentH, eyeRadius, 0x07FF);

            if (currentH > 25) {
                canvas.fillCircle(leftEyeX - (eyeWidth * 0.2f), eyeY + (currentH * 0.3f), 6, TFT_WHITE);
                canvas.fillCircle(rightEyeX - (eyeWidth * 0.2f), eyeY + (currentH * 0.3f), 6, TFT_WHITE);
            }
        }
    }

private:
    void renderShyLoveCard() {
        canvas.fillRoundRect(8, 150, 224, 82, 6, 0x0841);
        canvas.drawRoundRect(8, 150, 224, 82, 6, 0xF81F);
        canvas.drawRoundRect(10, 152, 220, 78, 4, 0x981F);

        canvas.setTextColor(0xF81F, 0x0841);
        canvas.setTextSize(1);
        canvas.drawCenterString("ROBOT SHY LOVE // 2s HOLD", 120, 156);

        int txtSize = (customShyText.length() > 14) ? 2 : 3;
        int textW = customShyText.length() * 6 * txtSize;
        int startX = max(16, 120 - (textW / 2));
        EmojiRenderer::renderTextWithEmojis(&canvas, customShyText, startX, 174, txtSize, 0xFFFF, 0x0841);

        uint32_t elapsed = millis() - shyLoveStartTime;
        float pct = 1.0f - ((float)elapsed / 5000.0f);
        if (pct < 0.0f) pct = 0.0f;
        if (pct > 1.0f) pct = 1.0f;
        canvas.drawRoundRect(28, 220, 184, 6, 2, 0xF81F);
        canvas.fillRect(30, 221, (int)(180 * pct), 4, 0xF81F);
    }

    void drawHappyEye(float cx, float cy, float w, uint16_t color) {
        for (int r = 0; r < 8; r++) {
            canvas.drawCircle(cx, cy + 10, (w / 2) - r, color);
        }
        // Mask the bottom half so only upward rainbow arc is visible
        canvas.fillRect(cx - w, cy + 10, w * 2, w, TFT_BLACK);
    }

    void drawAngryEye(float cx, float cy, float w, float h, bool isLeft, uint16_t color) {
        float x0 = cx - (w / 2);
        float y0 = cy - (h / 2);
        canvas.fillRoundRect(x0, y0, w, h, 12, color);
        
        // Slanted cut mask
        if (isLeft) {
            canvas.fillTriangle(x0 - 2, y0 - 2, x0 + w + 2, y0 - 2, x0 + w + 2, y0 + (h * 0.5f), TFT_BLACK);
        } else {
            canvas.fillTriangle(x0 - 2, y0 - 2, x0 + w + 2, y0 - 2, x0 - 2, y0 + (h * 0.5f), TFT_BLACK);
        }
    }

    void drawFloatingHeart(int x, int y, uint16_t color) {
        canvas.fillCircle(x - 5, y - 5, 5, color);
        canvas.fillCircle(x + 5, y - 5, 5, color);
        canvas.fillTriangle(x - 10, y - 4, x + 10, y - 4, x, y + 8, color);
    }
};

