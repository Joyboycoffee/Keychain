#pragma once
#include <LovyanGFX.hpp>
#include "display_setup.h"
#include "config.h"
#include "emoji_renderer.h"

enum DasaiState {
    DASAI_IDLE_LOOK    = 0, // Normal looking around & blinking (0-5s)
    DASAI_HAPPY_SQUINT = 1, // Happy upward curved arches (^ ^) + bounce (5-9s)
    DASAI_WINK         = 2, // Playful wink (^ o) (9-13s)
    DASAI_CURIOUS_TILT = 3, // Inquisitive glance up-right & down-left (13-17s)
    DASAI_SLEEPY       = 4, // Sleepy heavy eyelids & resting slits (- -) (17-22s)
    DASAI_SURPRISED    = 5, // Big excited round eyes (O O) (22-26s)
    DASAI_HEART_LOVE   = 6  // Heart eyes with blushing cheeks (26-30s)
};

class DasaiMochi {
public:
    DasaiState currentState = DASAI_IDLE_LOOK;
    int currentStyle = 0; // 0 = Warm White, 1 = Pastel Cyan, 2 = Sakura Pink, 3 = Mochi Gold
    bool isShyLoveActive = false;
    uint32_t shyLoveStartTime = 0;
    DasaiState preShyState = DASAI_IDLE_LOOK;
    String customShyText = "I LOVE YOU :heart: :sparkles:";

private:
    // Eye geometry
    float baseEyeWidth = 42.0f;
    float baseEyeHeight = 66.0f;
    float baseRadius = 18.0f;
    float eyeSpacing = 32.0f;

    // Smoothed interpolated coordinates & dimensions
    float curCenterX = 120.0f;
    float curCenterY = 110.0f;
    float targetCenterX = 120.0f;
    float targetCenterY = 110.0f;

    float curLeftH = 66.0f;
    float targetLeftH = 66.0f;
    float curRightH = 66.0f;
    float targetRightH = 66.0f;

    float curLeftW = 42.0f;
    float targetLeftW = 42.0f;
    float curRightW = 42.0f;
    float targetRightW = 42.0f;

    // Eyelid / arc squish factor (0.0 = normal pill, 1.0 = happy arc, -1.0 = sleepy slit)
    float curLeftArc = 0.0f;
    float targetLeftArc = 0.0f;
    float curRightArc = 0.0f;
    float targetRightArc = 0.0f;

    // Animation timers
    uint32_t stateStartTime = 0;
    uint32_t lastGazeChange = 0;
    uint32_t lastBlinkTime = 0;
    bool isBlinking = false;
    uint32_t sequenceStartTime = 0;
    int animPhase = 0;

public:
    DasaiMochi() {
        sequenceStartTime = millis();
        stateStartTime = millis();
    }

    void setStyle(int style) {
        currentStyle = (style >= 0 && style < 4) ? style : 0;
    }

    int cycleStyle() {
        currentStyle = (currentStyle + 1) % 4;
        return currentStyle;
    }

    uint16_t getEyeColor() {
        if (isShyLoveActive || currentState == DASAI_HEART_LOVE) return 0xF81F; // Pink Love
        if (currentStyle == 0) return 0xFFFF; // Warm Crisp White / OLED Ivory
        if (currentStyle == 1) return 0x7FFF; // Pastel Kawaii Cyan
        if (currentStyle == 2) return 0xFDF7; // Sakura Pink
        return 0xFFE0;                       // Mochi Gold
    }

    void setShyText(const String& txt) {
        if (txt.length() > 0) customShyText = txt;
    }

    void triggerShyLove(uint32_t durMs = 5000) {
        if (!isShyLoveActive) preShyState = currentState;
        isShyLoveActive = true;
        shyLoveStartTime = millis();
        currentState = DASAI_HEART_LOVE;
        targetCenterY = 92.0f; // Move up slightly for card
    }

    void triggerTap() {
        // Quick playful giggle bounce & happy smile
        currentState = DASAI_HAPPY_SQUINT;
        stateStartTime = millis();
        targetCenterY = 100.0f;
    }

    void update() {
        uint32_t now = millis();
        animPhase = (animPhase + 3) % 360;

        // Auto revert from shy love after 5 seconds
        if (isShyLoveActive && (now - shyLoveStartTime >= 5000)) {
            isShyLoveActive = false;
            currentState = preShyState;
            targetCenterY = 110.0f;
        }

        // =====================================================================
        // CHOREOGRAPHED SCENE SEQUENCER (~28s FULL CYCLE FROM RobotFace001.mp4)
        // =====================================================================
        if (!isShyLoveActive) {
            uint32_t cycleTime = (now - sequenceStartTime) % 28000;

            if (cycleTime < 5000) {
                // Scene 1: Neutral Idle & Saccade Looking Around (0-5s)
                currentState = DASAI_IDLE_LOOK;
                targetLeftH = baseEyeHeight;
                targetRightH = baseEyeHeight;
                targetLeftW = baseEyeWidth;
                targetRightW = baseEyeWidth;
                targetLeftArc = 0.0f;
                targetRightArc = 0.0f;

                // Saccade gaze shifting
                if (now - lastGazeChange > 1600 && !isBlinking) {
                    lastGazeChange = now;
                    int r = random(0, 4);
                    if (r == 0) { targetCenterX = 120.0f; targetCenterY = 110.0f; }
                    else if (r == 1) { targetCenterX = 102.0f; targetCenterY = 108.0f; } // glance left
                    else if (r == 2) { targetCenterX = 138.0f; targetCenterY = 108.0f; } // glance right
                    else { targetCenterX = 120.0f; targetCenterY = 116.0f; }             // glance down
                }

            } else if (cycleTime < 9500) {
                // Scene 2: Happy Mochi Smile & Double Giggle Bounce (^ ^) (5-9.5s)
                currentState = DASAI_HAPPY_SQUINT;
                targetCenterX = 120.0f;
                // Cute happy bobbing bounce
                float bounce = sin((now - 5000) * 0.008f) * 6.0f;
                targetCenterY = 110.0f - abs((int)bounce);
                targetLeftH = 36.0f;
                targetRightH = 36.0f;
                targetLeftW = 46.0f;
                targetRightW = 46.0f;
                targetLeftArc = 1.0f; // Happy smile arc
                targetRightArc = 1.0f;

            } else if (cycleTime < 13500) {
                // Scene 3: Playful Wink (^ o) (9.5-13.5s)
                currentState = DASAI_WINK;
                targetCenterX = 124.0f;
                targetCenterY = 110.0f;
                targetLeftH = 34.0f;
                targetLeftW = 46.0f;
                targetLeftArc = 1.0f; // Left eye winks
                targetRightH = 68.0f;
                targetRightW = 44.0f;
                targetRightArc = 0.0f; // Right eye looks alert

            } else if (cycleTime < 17500) {
                // Scene 4: Curious Head Tilt & Corner Glance (13.5-17.5s)
                currentState = DASAI_CURIOUS_TILT;
                targetCenterX = 134.0f;
                targetCenterY = 102.0f; // Look up-right
                targetLeftH = 64.0f;
                targetRightH = 64.0f;
                targetLeftW = 42.0f;
                targetRightW = 42.0f;
                targetLeftArc = 0.0f;
                targetRightArc = 0.0f;

            } else if (cycleTime < 22000) {
                // Scene 5: Sleepy Slits & Cozy Rest (- -) (17.5-22s)
                currentState = DASAI_SLEEPY;
                targetCenterX = 120.0f;
                targetCenterY = 114.0f;
                targetLeftH = 8.0f;
                targetRightH = 8.0f;
                targetLeftW = 44.0f;
                targetRightW = 44.0f;
                targetLeftArc = -1.0f; // Sleepy slit
                targetRightArc = -1.0f;

            } else if (cycleTime < 25500) {
                // Scene 6: Surprised / Excited Wide Eyes (O O) (22-25.5s)
                currentState = DASAI_SURPRISED;
                targetCenterX = 120.0f;
                targetCenterY = 108.0f;
                targetLeftH = 68.0f;
                targetRightH = 68.0f;
                targetLeftW = 50.0f;
                targetRightW = 50.0f;
                targetLeftArc = 0.0f;
                targetRightArc = 0.0f;

            } else {
                // Scene 7: Love Blush (<3 <3) (25.5-28s)
                currentState = DASAI_HEART_LOVE;
                targetCenterX = 120.0f;
                targetCenterY = 110.0f;
                targetLeftH = 58.0f;
                targetRightH = 58.0f;
                targetLeftW = 46.0f;
                targetRightW = 46.0f;
                targetLeftArc = 0.0f;
                targetRightArc = 0.0f;
            }
        }

        // =====================================================================
        // SMOOTH BLINKING ENGINE (NATURAL EYELID SQUISH & POP)
        // =====================================================================
        if (currentState == DASAI_IDLE_LOOK || currentState == DASAI_CURIOUS_TILT) {
            if (now - lastBlinkTime > 3400 && !isBlinking) {
                isBlinking = true;
                lastBlinkTime = now;
            } else if (isBlinking) {
                uint32_t blinkProgress = now - lastBlinkTime;
                if (blinkProgress < 80) {
                    targetLeftH = 4.0f;
                    targetRightH = 4.0f;
                } else if (blinkProgress < 170) {
                    targetLeftH = baseEyeHeight + 4.0f; // elastic overshoot
                    targetRightH = baseEyeHeight + 4.0f;
                } else {
                    isBlinking = false;
                    targetLeftH = baseEyeHeight;
                    targetRightH = baseEyeHeight;
                }
            }
        }

        // Gentle organic breathing bobbing
        float breathY = sin(now * 0.0025f) * 2.2f;

        // Exponential smoothing interpolation
        curCenterX += (targetCenterX - curCenterX) * 0.22f;
        curCenterY += ((targetCenterY + breathY) - curCenterY) * 0.22f;
        curLeftH   += (targetLeftH - curLeftH) * 0.35f;
        curRightH  += (targetRightH - curRightH) * 0.35f;
        curLeftW   += (targetLeftW - curLeftW) * 0.25f;
        curRightW  += (targetRightW - curRightW) * 0.25f;
        curLeftArc += (targetLeftArc - curLeftArc) * 0.3f;
        curRightArc+= (targetRightArc - curRightArc) * 0.3f;

        // =====================================================================
        // RENDER DASAI MOCHI TO CANVAS SPRITE
        // =====================================================================
        canvas.fillScreen(TFT_BLACK);

        uint16_t eyeColor = getEyeColor();
        float leftX = curCenterX - (curLeftW / 2.0f) - (eyeSpacing / 2.0f);
        float rightX = curCenterX + (curRightW / 2.0f) + (eyeSpacing / 2.0f);

        // 1. Draw Left Eye
        drawEyeUnit(leftX, curCenterY, curLeftW, curLeftH, curLeftArc, eyeColor, (currentState == DASAI_HEART_LOVE));

        // 2. Draw Right Eye
        drawEyeUnit(rightX, curCenterY, curRightW, curRightH, curRightArc, eyeColor, (currentState == DASAI_HEART_LOVE));

        // 3. Draw Blushing Cheeks (Active during Happy, Love, Wink, or Shy)
        if (isShyLoveActive || currentState == DASAI_HAPPY_SQUINT || currentState == DASAI_WINK || currentState == DASAI_HEART_LOVE) {
            float blushY = curCenterY + 36.0f;
            uint16_t blushCol = 0xF9AE; // Soft Kawaii Pink
            canvas.fillRoundRect((int)(leftX - 14), (int)blushY, 28, 10, 5, blushCol);
            canvas.fillRoundRect((int)(rightX - 14), (int)blushY, 28, 10, 5, blushCol);
        }

        // 4. Floating Hearts in Love State
        if (isShyLoveActive || currentState == DASAI_HEART_LOVE) {
            for (int i = 0; i < 4; i++) {
                int hx = 28 + i * 60 + (int)(sin((animPhase + i * 40) * 0.08f) * 6);
                int hy = 32 + (int)(cos((animPhase + i * 35) * 0.09f) * 8);
                drawFloatingHeart(hx, hy, 0xF81F);
            }
        }

        // 5. Floating "Z" Sleeping Bubbles in Sleepy State
        if (currentState == DASAI_SLEEPY) {
            int zPhase = (animPhase * 2) % 180;
            int zX = (int)rightX + 24 + (zPhase / 6);
            int zY = (int)curCenterY - 10 - (zPhase / 3);
            canvas.setTextColor(0x7FFF, TFT_BLACK);
            canvas.setTextSize(1);
            if (zPhase > 40) canvas.drawString("z", zX, zY);
            if (zPhase > 90) canvas.drawString("Z", zX + 8, zY - 12);
        }

        // 6. Shy Love Card Overlay if 2-Second Hold Active
        if (isShyLoveActive) {
            renderShyLoveCard();
        }
    }

private:
    void drawEyeUnit(float cx, float cy, float w, float h, float arcVal, uint16_t col, bool isHeart) {
        if (isHeart) {
            // Heart Eye
            drawFloatingHeart((int)cx, (int)cy, col, (int)(w * 0.5f));
            return;
        }

        if (arcVal > 0.4f) {
            // Happy Smile Arc (^): Upward curved arch
            float arcH = 26.0f;
            float topY = cy - 6.0f;
            for (int r = 0; r < 8; r++) {
                canvas.drawCircle((int)cx, (int)(topY + 12), (int)(w / 2) - r, col);
            }
            // Mask lower half so only upward rainbow arch is visible
            canvas.fillRect((int)(cx - w - 2), (int)(topY + 12), (int)(w * 2 + 4), (int)w, TFT_BLACK);
            return;
        }

        if (arcVal < -0.4f || h <= 8.0f) {
            // Sleepy / Blinking Horizontal Slit (-)
            canvas.fillRoundRect((int)(cx - (w / 2.0f)), (int)(cy - 3.0f), (int)w, 6, 3, col);
            return;
        }

        // Standard expressive rounded capsule
        int rx = (int)(cx - (w / 2.0f));
        int ry = (int)(cy - (h / 2.0f));
        int rad = (int)baseRadius;
        if (rad > (int)(w / 2)) rad = (int)(w / 2);
        if (rad > (int)(h / 2)) rad = (int)(h / 2);

        canvas.fillRoundRect(rx, ry, (int)w, (int)h, rad, col);

        // Glossy eye sparkle dot in top-left
        if (h > 30.0f) {
            canvas.fillCircle((int)(cx - (w * 0.22f)), (int)(cy - (h * 0.25f)), 4, TFT_WHITE);
        }
    }

    void drawFloatingHeart(int x, int y, uint16_t col, int sz = 6) {
        canvas.fillCircle(x - sz, y - sz, sz, col);
        canvas.fillCircle(x + sz, y - sz, sz, col);
        canvas.fillTriangle(x - (sz * 2), y - (sz / 2), x + (sz * 2), y - (sz / 2), x, y + (sz * 2), col);
    }

    void renderShyLoveCard() {
        canvas.fillRoundRect(8, 150, 224, 82, 6, 0x0841);
        canvas.drawRoundRect(8, 150, 224, 82, 6, 0xF81F);
        canvas.drawRoundRect(10, 152, 220, 78, 4, 0x981F);

        canvas.setTextColor(0xF81F, 0x0841);
        canvas.setTextSize(1);
        canvas.drawCenterString("DASAI MOCHI // 2s HOLD", 120, 156);

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
};
