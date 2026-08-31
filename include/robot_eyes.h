#pragma once
#include <LovyanGFX.hpp>
#include "display_setup.h"
#include "config.h"

class RobotEyes {
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

    PetMood currentMood = MOOD_IDLE;
    uint16_t eyeColor = 0x07FF; // Default Cyan (RGB565)

public:
    void setMood(PetMood mood) {
        currentMood = mood;
        if (mood == MOOD_HAPPY || mood == MOOD_LOVE) {
            eyeColor = 0xF81F; // Pink / Magenta
        } else if (mood == MOOD_ANGRY) {
            eyeColor = 0xF800; // Fierce Red
        } else {
            eyeColor = 0x07FF; // Glowing Cyan
        }
    }

    void update() {
        uint32_t now = millis();

        // 1. Random Gaze Tracking (looks around naturally)
        if (now - lastGazeChange > 2500 && !isBlinking) {
            lastGazeChange = now;
            targetX = 120 + random(-25, 26);
            targetY = 120 + random(-15, 16);
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

        if (currentMood == MOOD_HAPPY || currentMood == MOOD_LOVE) {
            // Draw Happy Upward Curved Eyes (Arcs / Rounded shapes)
            drawHappyEye(leftEyeX, currentY, eyeWidth, eyeColor);
            drawHappyEye(rightEyeX, currentY, eyeWidth, eyeColor);
            
            // Draw cute floating hearts
            drawFloatingHeart(120 + sin(now * 0.005f) * 40, 60 - ((now / 20) % 50), 0xF81F);
        } else if (currentMood == MOOD_ANGRY) {
            // Draw Fierce Slanted Angry Eyes
            drawAngryEye(leftEyeX, currentY, eyeWidth, currentH, true, eyeColor);
            drawAngryEye(rightEyeX, currentY, eyeWidth, currentH, false, eyeColor);
        } else {
            // Normal Expressive Glowing Eyes with Highlights
            canvas.fillRoundRect(leftEyeX - (eyeWidth / 2), eyeY, eyeWidth, currentH, eyeRadius, eyeColor);
            canvas.fillRoundRect(rightEyeX - (eyeWidth / 2), eyeY, eyeWidth, currentH, eyeRadius, eyeColor);

            // Cute Glossy Pupil Highlights (when eye is open)
            if (currentH > 25) {
                canvas.fillCircle(leftEyeX - (eyeWidth * 0.2f), eyeY + (currentH * 0.3f), 6, TFT_WHITE);
                canvas.fillCircle(rightEyeX - (eyeWidth * 0.2f), eyeY + (currentH * 0.3f), 6, TFT_WHITE);
            }
        }
    }

private:
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
