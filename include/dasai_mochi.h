#pragma once
#include <Arduino.h>
#include <LovyanGFX.hpp>
#include "display_setup.h"
#include "config.h"
#include "emoji_renderer.h"

extern LGFX_Sprite canvas;

// =========================================================================
// DASAI MOCHI COMPREHENSIVE EMOTION ENUM (23 ICONIC EXPRESSIONS & JDM MODES)
// =========================================================================
enum DasaiEmotion {
    DASAI_IDLE_LOOK      = 0,  // 0:  Master natural looking around & blinking loop
    DASAI_LOOK_LEFT      = 1,  // 1:  Look Left
    DASAI_LOOK_RIGHT     = 2,  // 2:  Look Right
    DASAI_LOOK_UP        = 3,  // 3:  Look Up / Serene
    DASAI_LOOK_DOWN      = 4,  // 4:  Look Down
    DASAI_HAPPY_SQUINT   = 5,  // 5:  Happy Smile / Squint
    DASAI_WINK_L         = 6,  // 6:  Playful Left Wink
    DASAI_WINK_R         = 7,  // 7:  Playful Right Wink
    DASAI_HEART_LOVE     = 8,  // 8:  Pulsing Heart Eyes & Love
    DASAI_SPARKLE_JOY    = 9,  // 9:  Diamond Sparkle Star Eyes
    DASAI_CURIOUS_TILT   = 10, // 10: Curious Tilted Brows
    DASAI_SURPRISED      = 11, // 11: Surprised / Shocked Wide Eyes
    DASAI_EXCITED_BOUNCE = 12, // 12: Dancing Party / Excited Bounce
    DASAI_SLEEPY_DROOP   = 13, // 13: Sleepy Droop / Drowsy Eyelids
    DASAI_SLEEPING_ZZZ   = 14, // 14: Sleeping Zzz & Yawn
    DASAI_ANGRY_GLARE    = 15, // 15: Slanted Angry Glare / Fierce
    DASAI_TURBO_RACE     = 16, // 16: Turbo JDM Speed Warp
    DASAI_DRIFT_G_FORCE  = 17, // 17: Drift G-Force & Tire Smoke
    DASAI_DIZZY_SPIRAL   = 18, // 18: Hypnotic Dizzy Spiral 360
    DASAI_SHOCKED_LIGHT  = 19, // 19: Demon Devil Horns / Shock
    DASAI_MATRIX_VISOR   = 20, // 20: Cyber Visor / Handsome Shades
    DASAI_SAD_TEAR       = 21, // 21: Sad Crying Teardrops
    DASAI_SMUG_CAT       = 22  // 22: Smug Cat :3 & Giggle
};

// Master Playback & Choreography Modes
enum DasaiCycleMode {
    CYCLE_AUTO_ALL   = 0, // Master Grand Loop through authentic expressions (72s)
    CYCLE_JDM_ACTION = 1, // Race action: Glare -> Turbo -> Drift -> Dizzy -> Devil
    CYCLE_KAWAII_CUTE= 2, // Kawaii loop: Blank -> Happy -> Sparkle -> Love -> Wink -> Giggle
    CYCLE_CHILL_RELAX= 3, // Relaxed lounge: Blank -> Drowsy -> Sleepy -> Yawn -> Smile
    CYCLE_MANUAL_LOCK= 4  // Locked to exact chosen emotion from Web Deck
};

class DasaiMochi {
public:
    DasaiEmotion   currentEmotion = DASAI_IDLE_LOOK;
    DasaiCycleMode cycleMode      = CYCLE_AUTO_ALL;
    int            currentStyle   = 0; // 0=Warm White, 1=Pastel Cyan, 2=Sakura Pink, 3=Mochi Gold, 4=Neon Violet
    uint16_t       primaryColor   = 0xFFFF; // Default Warm White

    // 2-Second Hold Secret Reaction Card
    bool         isShyLoveActive   = false;
    uint32_t     shyLoveStartTime  = 0;
    DasaiEmotion preShyEmotion     = DASAI_IDLE_LOOK;
    String       customShyText     = "I LOVE YOU :heart: :sparkles:";

private:
    // Physical geometry & physics
    float eyeWidth   = 56.0f;
    float eyeHeight  = 82.0f;
    float eyeRadius  = 22.0f;
    float eyeSpacing = 36.0f;

    float currentX = 120.0f;
    float currentY = 120.0f;
    float targetX  = 120.0f;
    float targetY  = 120.0f;

    float currentH = 82.0f;
    float targetH  = 82.0f;
    float currentW = 56.0f;
    float targetW  = 56.0f;

    uint32_t lastGazeChange = 0;
    uint32_t lastBlinkTime  = 0;
    bool     isBlinking     = false;
    uint32_t blinkStartTime = 0;

    int      animTick       = 0;
    uint32_t masterCycleStart = 0;
    uint32_t stateStartTime   = 0;

public:
    DasaiMochi() {
        masterCycleStart = millis();
        stateStartTime = millis();
        lastGazeChange = millis();
        lastBlinkTime = millis();
        setStyle(0);
    }

    void setStyle(int style) {
        currentStyle = (style >= 0 && style < 5) ? style : 0;
        switch (currentStyle) {
            case 0: primaryColor = 0xFFFF; break; // Crisp Snow White
            case 1: primaryColor = 0x7FFF; break; // Pastel Cyan
            case 2: primaryColor = 0xF81F; break; // Sakura Pink
            case 3: primaryColor = 0xFFE0; break; // Mochi Gold
            case 4: primaryColor = 0xD69A; break; // Neon Violet
        }
    }

    int cycleStyle() {
        setStyle((currentStyle + 1) % 5);
        return currentStyle;
    }

    void setShyText(const String& txt) {
        if (txt.length() > 0) customShyText = txt;
    }

    void triggerShyLove(uint32_t durMs = 5000) {
        if (!isShyLoveActive) preShyEmotion = currentEmotion;
        isShyLoveActive = true;
        shyLoveStartTime = millis();
        currentEmotion = DASAI_HEART_LOVE;
    }

    void triggerTap() {
        currentEmotion = (currentEmotion == DASAI_HAPPY_SQUINT) ? DASAI_EXCITED_BOUNCE : DASAI_HAPPY_SQUINT;
        cycleMode = CYCLE_MANUAL_LOCK;
        stateStartTime = millis();
    }

    void triggerTripleTapTurbo() {
        currentEmotion = DASAI_TURBO_RACE;
        cycleMode = CYCLE_MANUAL_LOCK;
        stateStartTime = millis();
    }

    void setEmotion(DasaiEmotion emo) {
        currentEmotion = emo;
        cycleMode = CYCLE_MANUAL_LOCK;
        stateStartTime = millis();
        isBlinking = false;
    }

    void setCycleMode(DasaiCycleMode mode) {
        cycleMode = mode;
        masterCycleStart = millis();
        stateStartTime = millis();
    }

    void update() {
        uint32_t now = millis();
        animTick++;

        // Auto revert from shy love after 5 seconds
        if (isShyLoveActive && (now - shyLoveStartTime >= 5000)) {
            isShyLoveActive = false;
            currentEmotion = preShyEmotion;
        }

        // =====================================================================
        // CHOREOGRAPHY CYCLE STATE MACHINE
        // =====================================================================
        if (!isShyLoveActive && cycleMode != CYCLE_MANUAL_LOCK) {
            executeMasterCycle(now);
        }

        // =====================================================================
        // 60 FPS ORGANIC GAZE & SACCADES PHYSICS
        // =====================================================================
        if (currentEmotion == DASAI_IDLE_LOOK) {
            // Random natural gaze tracking
            if (now - lastGazeChange > 2600 && !isBlinking) {
                lastGazeChange = now;
                int r = random(0, 10);
                if (r < 5) {
                    targetX = 120.0f; targetY = 120.0f; // Center
                } else if (r < 7) {
                    targetX = 120.0f + random(-28, -12); targetY = 120.0f + random(-8, 8); // Look Left
                } else if (r < 9) {
                    targetX = 120.0f + random(12, 28); targetY = 120.0f + random(-8, 8);  // Look Right
                } else {
                    targetX = 120.0f; targetY = 120.0f - random(10, 20); // Look Up
                }
            }
        } else if (currentEmotion == DASAI_LOOK_LEFT) {
            targetX = 85.0f; targetY = 120.0f;
        } else if (currentEmotion == DASAI_LOOK_RIGHT) {
            targetX = 155.0f; targetY = 120.0f;
        } else if (currentEmotion == DASAI_LOOK_UP) {
            targetX = 120.0f; targetY = 95.0f;
        } else if (currentEmotion == DASAI_LOOK_DOWN) {
            targetX = 120.0f; targetY = 145.0f;
        } else {
            targetX = 120.0f;
            targetY = isShyLoveActive ? 90.0f : 120.0f;
        }

        // =====================================================================
        // ORGANIC BLINKING ENGINE (NATURAL CURVED CLOSURE)
        // =====================================================================
        bool canBlink = (currentEmotion == DASAI_IDLE_LOOK || currentEmotion == DASAI_LOOK_LEFT || 
                         currentEmotion == DASAI_LOOK_RIGHT || currentEmotion == DASAI_LOOK_UP || 
                         currentEmotion == DASAI_LOOK_DOWN || currentEmotion == DASAI_CURIOUS_TILT);

        if (canBlink) {
            if (!isBlinking && (now - lastBlinkTime > 3400 + (uint32_t)random(0, 1800))) {
                isBlinking = true;
                blinkStartTime = now;
                lastBlinkTime = now;
            } else if (isBlinking) {
                uint32_t bElapsed = now - blinkStartTime;
                if (bElapsed < 110) {
                    // Closing eyelid
                    targetH = 4.0f;
                } else if (bElapsed < 220) {
                    // Opening eyelid
                    targetH = eyeHeight;
                } else {
                    isBlinking = false;
                    targetH = eyeHeight;
                }
            } else {
                targetH = eyeHeight;
            }
        } else {
            isBlinking = false;
            targetH = eyeHeight;
        }

        // Gentle breathing bobbing (sine wave ±3px)
        float breathBob = sin((float)animTick * 0.06f) * 3.0f;

        // Smooth Exponential Interpolation (Easing)
        currentX += (targetX - currentX) * 0.22f;
        currentY += (targetY + breathBob - currentY) * 0.22f;
        currentH += (targetH - currentH) * 0.35f;
        currentW += (targetW - currentW) * 0.25f;

        // =====================================================================
        // RENDER NATIVE 60 FPS VECTOR GRAPHICS TO CANVAS
        // =====================================================================
        canvas.fillScreen(TFT_BLACK);

        float leftEyeX  = currentX - (currentW / 2.0f) - (eyeSpacing / 2.0f);
        float rightEyeX = currentX + (currentW / 2.0f) + (eyeSpacing / 2.0f);
        float eyeY      = currentY - (currentH / 2.0f);

        uint16_t activeCol = isShyLoveActive ? 0xF81F : primaryColor;

        // Dispatch to emotion renderer
        switch (currentEmotion) {
            case DASAI_IDLE_LOOK:
            case DASAI_LOOK_LEFT:
            case DASAI_LOOK_RIGHT:
            case DASAI_LOOK_UP:
            case DASAI_LOOK_DOWN:
                renderClassicCapsuleEyes(leftEyeX, rightEyeX, eyeY, currentW, currentH, activeCol);
                break;

            case DASAI_HAPPY_SQUINT:
                renderHappySmile(leftEyeX, rightEyeX, currentY, currentW, activeCol);
                break;

            case DASAI_WINK_L:
                renderWink(leftEyeX, rightEyeX, eyeY, currentY, currentW, currentH, true, activeCol);
                break;

            case DASAI_WINK_R:
                renderWink(leftEyeX, rightEyeX, eyeY, currentY, currentW, currentH, false, activeCol);
                break;

            case DASAI_HEART_LOVE:
                renderHeartEyes(leftEyeX, rightEyeX, currentY, currentW, activeCol);
                break;

            case DASAI_SPARKLE_JOY:
                renderSparkleEyes(leftEyeX, rightEyeX, currentY, currentW, activeCol);
                break;

            case DASAI_CURIOUS_TILT:
                renderCurious(leftEyeX, rightEyeX, eyeY, currentW, currentH, activeCol);
                break;

            case DASAI_SURPRISED:
                renderSurprised(leftEyeX, rightEyeX, eyeY, currentW, currentH, activeCol);
                break;

            case DASAI_EXCITED_BOUNCE:
                renderExcitedBounce(leftEyeX, rightEyeX, currentY, currentW, activeCol);
                break;

            case DASAI_SLEEPY_DROOP:
                renderSleepyDroop(leftEyeX, rightEyeX, eyeY, currentW, currentH, activeCol);
                break;

            case DASAI_SLEEPING_ZZZ:
                renderSleepingZzz(leftEyeX, rightEyeX, currentY, currentW, activeCol);
                break;

            case DASAI_ANGRY_GLARE:
                renderAngryGlare(leftEyeX, rightEyeX, eyeY, currentW, currentH, activeCol);
                break;

            case DASAI_TURBO_RACE:
                renderTurboRace(leftEyeX, rightEyeX, eyeY, currentW, currentH, activeCol);
                break;

            case DASAI_DRIFT_G_FORCE:
                renderDriftGForce(leftEyeX, rightEyeX, eyeY, currentW, currentH, activeCol);
                break;

            case DASAI_DIZZY_SPIRAL:
                renderDizzySpiral(leftEyeX, rightEyeX, currentY, currentW, activeCol);
                break;

            case DASAI_SHOCKED_LIGHT:
                renderDevilShock(leftEyeX, rightEyeX, eyeY, currentW, currentH, activeCol);
                break;

            case DASAI_MATRIX_VISOR:
                renderMatrixVisor(currentX, currentY, activeCol);
                break;

            case DASAI_SAD_TEAR:
                renderSadTears(leftEyeX, rightEyeX, eyeY, currentW, currentH, activeCol);
                break;

            case DASAI_SMUG_CAT:
                renderSmugCat(leftEyeX, rightEyeX, currentY, currentW, activeCol);
                break;

            default:
                renderClassicCapsuleEyes(leftEyeX, rightEyeX, eyeY, currentW, currentH, activeCol);
                break;
        }

        // Render Shy Love 2s Hold card overlay if active
        if (isShyLoveActive) {
            renderShyLoveCard();
        }
    }

private:
    // =========================================================================
    // HIGH-DPI NATIVE EMOTION RENDERERS
    // =========================================================================

    // 1. CLASSIC DASAI CAPSULE PILL EYES (Ultra-smooth antialiased with double gloss highlights)
    void renderClassicCapsuleEyes(float lx, float rx, float y, float w, float h, uint16_t col) {
        float r = min(eyeRadius, h / 2.0f);
        if (r < 2.0f) r = 2.0f;

        // Outer Glow contour
        canvas.fillRoundRect((int)(lx - w/2.0f - 1), (int)(y - 1), (int)(w + 2), (int)(h + 2), (int)(r + 1), 0x2104);
        canvas.fillRoundRect((int)(rx - w/2.0f - 1), (int)(y - 1), (int)(w + 2), (int)(h + 2), (int)(r + 1), 0x2104);

        // Solid Body
        canvas.fillRoundRect((int)(lx - w/2.0f), (int)y, (int)w, (int)h, (int)r, col);
        canvas.fillRoundRect((int)(rx - w/2.0f), (int)y, (int)w, (int)h, (int)r, col);

        // Glossy White Eye Reflection Highlights (Large top-left orb + micro bottom-right sparkle)
        if (h > 30.0f) {
            int hlR = max(4, (int)(w * 0.16f));
            canvas.fillCircle((int)(lx - (w * 0.18f)), (int)(y + (h * 0.28f)), hlR, TFT_WHITE);
            canvas.fillCircle((int)(rx - (w * 0.18f)), (int)(y + (h * 0.28f)), hlR, TFT_WHITE);

            // Secondary micro highlight
            canvas.fillCircle((int)(lx + (w * 0.20f)), (int)(y + (h * 0.65f)), 3, TFT_WHITE);
            canvas.fillCircle((int)(rx + (w * 0.20f)), (int)(y + (h * 0.65f)), 3, TFT_WHITE);
        }
    }

    // 2. HAPPY SMILE / SQUINT (Upward curved rainbow arcs + rosy blush cheeks)
    void renderHappySmile(float lx, float rx, float cy, float w, uint16_t col) {
        float bounce = sin((float)animTick * 0.15f) * 4.0f;
        float drawY = cy + bounce;

        // Draw thick smooth upward crescent arcs
        for (int t = 0; t < 10; t++) {
            canvas.drawCircle((int)lx, (int)(drawY + 12), (int)((w / 2.0f) - t), col);
            canvas.drawCircle((int)rx, (int)(drawY + 12), (int)((w / 2.0f) - t), col);
        }
        // Mask bottom half
        canvas.fillRect((int)(lx - w - 4), (int)(drawY + 12), (int)(w * 2 + 8), (int)(w + 10), TFT_BLACK);
        canvas.fillRect((int)(rx - w - 4), (int)(drawY + 12), (int)(w * 2 + 8), (int)(w + 10), TFT_BLACK);

        // Rosy Blushing Cheeks
        renderCheekBlush((int)drawY + 16);
    }

    // 3. WINK (One glossy eye + one wink crescent with sparkle)
    void renderWink(float lx, float rx, float y, float cy, float w, float h, bool isLeft, uint16_t col) {
        if (isLeft) {
            // Left eye winks, Right eye open
            for (int t = 0; t < 9; t++) {
                canvas.drawCircle((int)lx, (int)(cy + 10), (int)((w / 2.0f) - t), col);
            }
            canvas.fillRect((int)(lx - w - 4), (int)(cy + 10), (int)(w * 2 + 8), (int)(w + 10), TFT_BLACK);

            // Star sparkle near wink
            drawSparkleStar((int)(lx - 32), (int)(cy - 8), 12, col);

            // Right eye open
            canvas.fillRoundRect((int)(rx - w/2.0f), (int)y, (int)w, (int)h, (int)eyeRadius, col);
            if (h > 30.0f) {
                canvas.fillCircle((int)(rx - (w * 0.18f)), (int)(y + (h * 0.28f)), 7, TFT_WHITE);
            }
        } else {
            // Left eye open, Right eye winks
            canvas.fillRoundRect((int)(lx - w/2.0f), (int)y, (int)w, (int)h, (int)eyeRadius, col);
            if (h > 30.0f) {
                canvas.fillCircle((int)(lx - (w * 0.18f)), (int)(y + (h * 0.28f)), 7, TFT_WHITE);
            }

            for (int t = 0; t < 9; t++) {
                canvas.drawCircle((int)rx, (int)(cy + 10), (int)((w / 2.0f) - t), col);
            }
            canvas.fillRect((int)(rx - w - 4), (int)(cy + 10), (int)(w * 2 + 8), (int)(w + 10), TFT_BLACK);

            drawSparkleStar((int)(rx + 32), (int)(cy - 8), 12, col);
        }

        renderCheekBlush((int)cy + 20);
    }

    // 4. HEART LOVE (Pulsing beating heart eyes & floating heart particles)
    void renderHeartEyes(float lx, float rx, float cy, float w, uint16_t col) {
        float pulse = 1.0f + (sin((float)animTick * 0.18f) * 0.15f);
        int heartScale = (int)(22 * pulse);

        drawHeart((int)lx, (int)cy, heartScale, col);
        drawHeart((int)rx, (int)cy, heartScale, col);

        // Floating upward animated heart particles
        for (int i = 0; i < 4; i++) {
            int hx = 40 + i * 52 + (int)(sin((float)(animTick + i * 40) * 0.08f) * 10);
            int hy = (240 - ((animTick * 2 + i * 60) % 240));
            drawFloatingHeart(hx, hy, 0xF81F);
        }

        renderCheekBlush((int)cy + 26);
    }

    // 5. SPARKLE JOY (4-point diamond star eyes & rotating twinkles)
    void renderSparkleEyes(float lx, float rx, float cy, float w, uint16_t col) {
        float pulse = 1.0f + (sin((float)animTick * 0.20f) * 0.12f);
        int starSize = (int)(32 * pulse);

        drawSparkleStar((int)lx, (int)cy, starSize, col);
        drawSparkleStar((int)rx, (int)cy, starSize, col);

        // Orbiting micro stars
        for (int i = 0; i < 4; i++) {
            float angle = (animTick * 0.05f) + (i * 1.57f);
            int ox = 120 + (int)(cos(angle) * 88);
            int oy = (int)cy + (int)(sin(angle) * 35);
            drawSparkleStar(ox, oy, 8, TFT_WHITE);
        }

        renderCheekBlush((int)cy + 22);
    }

    // 6. CURIOUS TILT (Asymmetric raised brows & questioning gaze)
    void renderCurious(float lx, float rx, float y, float w, float h, uint16_t col) {
        // Left eye normal, Right eye raised higher and wider
        canvas.fillRoundRect((int)(lx - w/2.0f), (int)y + 6, (int)w, (int)(h * 0.9f), (int)eyeRadius, col);
        canvas.fillCircle((int)(lx - (w * 0.18f)), (int)(y + (h * 0.32f)), 6, TFT_WHITE);

        canvas.fillRoundRect((int)(rx - (w * 1.1f) / 2.0f), (int)y - 12, (int)(w * 1.1f), (int)(h * 1.05f), (int)eyeRadius, col);
        canvas.fillCircle((int)(rx - (w * 0.18f)), (int)(y - 12 + (h * 0.28f)), 8, TFT_WHITE);

        // Question mark / sweat drop accent
        canvas.drawCircle((int)(rx + 42), (int)(y - 10), 5, col);
        canvas.drawFastVLine((int)(rx + 42), (int)(y - 5), 8, col);
        canvas.fillCircle((int)(rx + 42), (int)(y + 7), 2, col);
    }

    // 7. SURPRISED / SHOCKED (Giant wide circles + tiny shocked pupils)
    void renderSurprised(float lx, float rx, float y, float w, float h, uint16_t col) {
        // Trembling vibration
        int vibX = random(-1, 2);
        int vibY = random(-1, 2);

        canvas.fillCircle((int)lx + vibX, (int)(y + h/2.0f) + vibY, (int)(w * 0.65f), col);
        canvas.fillCircle((int)rx + vibX, (int)(y + h/2.0f) + vibY, (int)(w * 0.65f), col);

        // Tiny pinpoint shock pupils
        canvas.fillCircle((int)lx + vibX, (int)(y + h/2.0f) + vibY, 5, TFT_BLACK);
        canvas.fillCircle((int)rx + vibX, (int)(y + h/2.0f) + vibY, 5, TFT_BLACK);
        canvas.fillCircle((int)lx + vibX + 1, (int)(y + h/2.0f) + vibY - 1, 2, TFT_WHITE);
        canvas.fillCircle((int)rx + vibX + 1, (int)(y + h/2.0f) + vibY - 1, 2, TFT_WHITE);
    }

    // 8. EXCITED BOUNCE / DANCE (Bouncing crescents + musical notes)
    void renderExcitedBounce(float lx, float rx, float cy, float w, uint16_t col) {
        float bounce = fabs(sin((float)animTick * 0.22f)) * 18.0f;
        float drawY = cy - bounce;

        for (int t = 0; t < 10; t++) {
            canvas.drawCircle((int)lx, (int)(drawY + 12), (int)((w / 2.0f) - t), col);
            canvas.drawCircle((int)rx, (int)(drawY + 12), (int)((w / 2.0f) - t), col);
        }
        canvas.fillRect((int)(lx - w - 4), (int)(drawY + 12), (int)(w * 2 + 8), (int)(w + 10), TFT_BLACK);
        canvas.fillRect((int)(rx - w - 4), (int)(drawY + 12), (int)(w * 2 + 8), (int)(w + 10), TFT_BLACK);

        // Musical note particles
        int noteX = 35 + ((animTick * 3) % 170);
        int noteY = 55 + (int)(sin((float)animTick * 0.1f) * 15);
        drawMusicalNote(noteX, noteY, col);

        renderCheekBlush((int)drawY + 16);
    }

    // 9. SLEEPY DROOP (Half-closed heavy eyelids)
    void renderSleepyDroop(float lx, float rx, float y, float w, float h, uint16_t col) {
        float droopH = h * 0.45f;
        float droopY = y + (h * 0.40f);

        canvas.fillRoundRect((int)(lx - w/2.0f), (int)droopY, (int)w, (int)droopH, 8, col);
        canvas.fillRoundRect((int)(rx - w/2.0f), (int)droopY, (int)w, (int)droopH, 8, col);

        // Heavy slanted eyelid crease
        canvas.drawLine((int)(lx - w/2.0f - 4), (int)droopY, (int)(lx + w/2.0f + 4), (int)(droopY + 6), 0x39E7);
        canvas.drawLine((int)(rx - w/2.0f - 4), (int)(droopY + 6), (int)(rx + w/2.0f + 4), (int)droopY, 0x39E7);
    }

    // 10. SLEEPING ZZZ (Sleeping curved slits & floating Zzz bubbles)
    void renderSleepingZzz(float lx, float rx, float cy, float w, uint16_t col) {
        // Sleeping eye arcs (downward curved)
        for (int t = 0; t < 6; t++) {
            canvas.drawCircle((int)lx, (int)(cy - 8), (int)((w / 2.0f) - t), col);
            canvas.drawCircle((int)rx, (int)(cy - 8), (int)((w / 2.0f) - t), col);
        }
        canvas.fillRect((int)(lx - w - 4), (int)(cy - w - 12), (int)(w * 2 + 8), (int)(w + 10), TFT_BLACK);
        canvas.fillRect((int)(rx - w - 4), (int)(cy - w - 12), (int)(w * 2 + 8), (int)(w + 10), TFT_BLACK);

        // Animated floating Zzz characters drifting upward
        for (int i = 0; i < 3; i++) {
            int zProgress = (animTick + i * 40) % 120;
            int zX = 145 + (int)(zProgress * 0.6f);
            int zY = (int)cy - 15 - zProgress;
            int zSz = 1 + (i % 2);
            canvas.setTextColor(0x7FFF, TFT_BLACK);
            canvas.setTextSize(zSz);
            canvas.drawString("Z", zX, zY);
        }
    }

    // 11. ANGRY GLARE (Sharp slanted brows & fierce fiery glow)
    void renderAngryGlare(float lx, float rx, float y, float w, float h, uint16_t col) {
        uint16_t angryCol = (primaryColor == 0xFFFF) ? 0xF800 : primaryColor;

        // Slanted cut boxes
        canvas.fillRoundRect((int)(lx - w/2.0f), (int)y, (int)w, (int)h, 12, angryCol);
        canvas.fillRoundRect((int)(rx - w/2.0f), (int)y, (int)w, (int)h, 12, angryCol);

        // Cut triangles for fierce angle
        canvas.fillTriangle((int)(lx - w/2.0f - 2), (int)(y - 2), 
                            (int)(lx + w/2.0f + 2), (int)(y - 2), 
                            (int)(lx + w/2.0f + 2), (int)(y + (h * 0.55f)), TFT_BLACK);

        canvas.fillTriangle((int)(rx - w/2.0f - 2), (int)(y - 2), 
                            (int)(rx + w/2.0f + 2), (int)(y - 2), 
                            (int)(rx - w/2.0f - 2), (int)(y + (h * 0.55f)), TFT_BLACK);

        // Fiery brow highlights
        canvas.drawLine((int)(lx - w/2.0f), (int)y, (int)(lx + w/2.0f), (int)(y + (h * 0.55f)), 0xFFE0);
        canvas.drawLine((int)(rx + w/2.0f), (int)y, (int)(rx - w/2.0f), (int)(y + (h * 0.55f)), 0xFFE0);
    }

    // 12. TURBO RACE (Aerodynamic speed eyes + streaming horizontal wind streaks)
    void renderTurboRace(float lx, float rx, float y, float w, float h, uint16_t col) {
        // Forward aerodynamic lean
        renderAngryGlare(lx + 8, rx + 8, y, w, h, col);

        // High-speed wind warp streaks
        for (int i = 0; i < 7; i++) {
            int lineY = 40 + i * 26;
            int speedX = (animTick * 18 + i * 45) % 280;
            int lineX = 240 - speedX;
            canvas.drawFastHLine(lineX, lineY, 45 + (i * 8), 0x7FFF);
            canvas.drawFastHLine(lineX + 10, lineY + 1, 25, TFT_WHITE);
        }

        // Nitro exhaust flames at bottom
        for (int f = 0; f < 5; f++) {
            int fx = 30 + f * 45;
            int fh = 8 + (int)(sin((float)(animTick + f * 20) * 0.3f) * 6);
            canvas.fillTriangle(fx, 238, fx + 15, 238, fx + 7, 238 - fh, 0xFD20);
        }
    }

    // 13. DRIFT G-FORCE (Sideways intense gaze + billowing tire smoke particles)
    void renderDriftGForce(float lx, float rx, float y, float w, float h, uint16_t col) {
        // Hard right gaze
        renderClassicCapsuleEyes(lx + 22, rx + 22, y, w, h, col);

        // Billowing tire smoke clouds drifting across bottom
        for (int s = 0; s < 6; s++) {
            int smokeProgress = (animTick * 3 + s * 45) % 260;
            int sx = smokeProgress;
            int sy = 220 - (int)(sin((float)smokeProgress * 0.05f) * 15) - (s * 4);
            int sR = 12 + (s * 3);
            canvas.fillCircle(sx, sy, sR, 0x39E7);
            canvas.drawCircle(sx, sy, sR, 0x7BEF);
        }
    }

    // 14. DIZZY SPIRAL (60 FPS hypnotic rotating spiral coils)
    void renderDizzySpiral(float lx, float rx, float cy, float w, uint16_t col) {
        float rotAngle = (float)animTick * 0.12f;

        drawSpiral((int)lx, (int)cy, (int)(w * 0.55f), rotAngle, col);
        drawSpiral((int)rx, (int)cy, (int)(w * 0.55f), rotAngle, col);

        // Circling dizzy cartoon stars
        for (int s = 0; s < 3; s++) {
            float sa = rotAngle + (s * 2.09f);
            int starX = 120 + (int)(cos(sa) * 90);
            int starY = (int)cy - 40 + (int)(sin(sa) * 18);
            drawSparkleStar(starX, starY, 9, 0xFFE0);
        }
    }

    // 15. DEVIL / SHOCKED (Demon horn silhouettes + electric lightning sparks)
    void renderDevilShock(float lx, float rx, float y, float w, float h, uint16_t col) {
        renderAngryGlare(lx, rx, y + 10, w, h, col);

        // Glowing Demon Horns above eyes
        canvas.fillTriangle((int)(lx - w/2.0f), (int)y + 6, (int)(lx - w/2.0f + 16), (int)y + 6, (int)(lx - w/2.0f - 8), (int)y - 28, 0xF800);
        canvas.fillTriangle((int)(rx + w/2.0f - 16), (int)y + 6, (int)(rx + w/2.0f), (int)y + 6, (int)(rx + w/2.0f + 8), (int)y - 28, 0xF800);

        // Electric lightning sparks
        if (random(0, 3) == 0) {
            int lx0 = random(20, 220);
            int ly0 = random(20, 100);
            canvas.drawLine(lx0, ly0, lx0 + 10, ly0 + 15, 0x7FFF);
            canvas.drawLine(lx0 + 10, ly0 + 15, lx0 + 5, ly0 + 20, 0x7FFF);
            canvas.drawLine(lx0 + 5, ly0 + 20, lx0 + 18, ly0 + 35, TFT_WHITE);
        }
    }

    // 16. MATRIX VISOR / HANDSOME (Cyberpunk holographic shades & equalizer)
    void renderMatrixVisor(float cx, float cy, uint16_t col) {
        // Large cool sunglasses visor
        int visorW = 204;
        int visorH = 68;
        int vx = (int)(cx - (visorW / 2));
        int vy = (int)(cy - (visorH / 2));

        canvas.fillRoundRect(vx, vy, visorW, visorH, 16, 0x0841);
        canvas.drawRoundRect(vx, vy, visorW, visorH, 16, col);
        canvas.drawRoundRect(vx + 2, vy + 2, visorW - 4, visorH - 4, 14, 0x2104);

        // Central bridge
        canvas.fillRect((int)cx - 10, vy + 4, 20, 10, 0x0000);
        canvas.drawFastHLine((int)cx - 10, vy + 4, 20, col);

        // Animated Equalizer Bars across the visor
        for (int b = 0; b < 18; b++) {
            int bx = vx + 14 + (b * 10);
            int bHeight = 8 + (int)(fabs(sin((float)(animTick + b * 15) * 0.15f)) * 40.0f);
            canvas.fillRect(bx, (int)cy + (visorH / 2) - 8 - bHeight, 6, bHeight, col);
            canvas.fillRect(bx, (int)cy + (visorH / 2) - 8 - bHeight, 6, 2, TFT_WHITE);
        }
    }

    // 17. SAD TEARS (Tearful eyes & animated dripping tears)
    void renderSadTears(float lx, float rx, float y, float w, float h, uint16_t col) {
        // Droopy sad eyes
        canvas.fillRoundRect((int)(lx - w/2.0f), (int)y, (int)w, (int)h, (int)eyeRadius, col);
        canvas.fillRoundRect((int)(rx - w/2.0f), (int)y, (int)w, (int)h, (int)eyeRadius, col);

        // Downward slanted sad eyebrows
        canvas.fillTriangle((int)(lx - w/2.0f - 2), (int)y - 2, 
                            (int)(lx + w/2.0f + 2), (int)y - 2, 
                            (int)(lx - w/2.0f - 2), (int)(y + (h * 0.40f)), TFT_BLACK);

        canvas.fillTriangle((int)(rx - w/2.0f - 2), (int)y - 2, 
                            (int)(rx + w/2.0f + 2), (int)y - 2, 
                            (int)(rx + w/2.0f + 2), (int)(y + (h * 0.40f)), TFT_BLACK);

        // Animated falling teardrops
        int tear1Y = (animTick * 3) % 110;
        int tear2Y = (animTick * 3 + 55) % 110;
        drawTeardrop((int)lx + 12, (int)(y + h - 6) + tear1Y, 0x7FFF);
        drawTeardrop((int)rx - 12, (int)(y + h - 6) + tear2Y, 0x7FFF);
    }

    // 18. SMUG CAT (Cat smug eyes > < with :3 mouth)
    void renderSmugCat(float lx, float rx, float cy, float w, uint16_t col) {
        float bounce = sin((float)animTick * 0.12f) * 3.0f;
        float drawY = cy + bounce;

        // Draw > < smug cat angles
        for (int t = 0; t < 6; t++) {
            // Left eye >
            canvas.drawLine((int)(lx - w/2.0f), (int)(drawY - 18 + t), (int)(lx + w/2.0f), (int)(drawY + t), col);
            canvas.drawLine((int)(lx + w/2.0f), (int)(drawY + t), (int)(lx - w/2.0f), (int)(drawY + 18 + t), col);

            // Right eye <
            canvas.drawLine((int)(rx + w/2.0f), (int)(drawY - 18 + t), (int)(rx - w/2.0f), (int)(drawY + t), col);
            canvas.drawLine((int)(rx - w/2.0f), (int)(drawY + t), (int)(rx + w/2.0f), (int)(drawY + 18 + t), col);
        }

        // Cute :3 Cat Mouth
        int mx = (int)((lx + rx) / 2.0f);
        int my = (int)drawY + 28;
        for (int t = 0; t < 3; t++) {
            canvas.drawCircle(mx - 8, my, 8 - t, col);
            canvas.drawCircle(mx + 8, my, 8 - t, col);
        }
        canvas.fillRect(mx - 20, my - 12, 40, 12, TFT_BLACK);

        renderCheekBlush((int)drawY + 14);
    }

    // =========================================================================
    // GRAPHICS HELPER UTILITIES
    // =========================================================================
    void renderCheekBlush(int y) {
        canvas.fillRoundRect(18, y, 32, 10, 5, 0xF81F);
        canvas.fillRoundRect(190, y, 32, 10, 5, 0xF81F);
    }

    void drawHeart(int cx, int cy, int size, uint16_t color) {
        int r = size / 2;
        canvas.fillCircle(cx - r/2, cy - r/2, r/2, color);
        canvas.fillCircle(cx + r/2, cy - r/2, r/2, color);
        canvas.fillTriangle(cx - r, cy - r/4, cx + r, cy - r/4, cx, cy + r, color);
    }

    void drawFloatingHeart(int x, int y, uint16_t color) {
        canvas.fillCircle(x - 4, y - 4, 4, color);
        canvas.fillCircle(x + 4, y - 4, 4, color);
        canvas.fillTriangle(x - 8, y - 3, x + 8, y - 3, x, y + 7, color);
    }

    void drawSparkleStar(int cx, int cy, int size, uint16_t color) {
        int half = size / 2;
        int quarter = size / 4;
        canvas.fillTriangle(cx, cy - half, cx - quarter, cy, cx + quarter, cy, color);
        canvas.fillTriangle(cx, cy + half, cx - quarter, cy, cx + quarter, cy, color);
        canvas.fillTriangle(cx - half, cy, cx, cy - quarter, cx, cy + quarter, color);
        canvas.fillTriangle(cx + half, cy, cx, cy - quarter, cx, cy + quarter, color);
        canvas.fillCircle(cx, cy, max(2, quarter / 2), TFT_WHITE);
    }

    void drawSpiral(int cx, int cy, int maxR, float rot, uint16_t color) {
        for (float r = 4; r < maxR; r += 1.5f) {
            float a = rot + (r * 0.45f);
            int px = cx + (int)(cos(a) * r);
            int py = cy + (int)(sin(a) * r);
            canvas.fillCircle(px, py, 2, color);
        }
    }

    void drawTeardrop(int x, int y, uint16_t color) {
        if (y < 235) {
            canvas.fillCircle(x, y, 4, color);
            canvas.fillTriangle(x - 4, y, x + 4, y, x, y - 8, color);
        }
    }

    void drawMusicalNote(int x, int y, uint16_t color) {
        canvas.fillCircle(x, y, 4, color);
        canvas.fillCircle(x + 12, y - 3, 4, color);
        canvas.drawFastVLine(x + 3, y - 14, 14, color);
        canvas.drawFastVLine(x + 15, y - 17, 14, color);
        canvas.drawFastHLine(x + 3, y - 14, 12, color);
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

    // =========================================================================
    // CHOREOGRAPHED CYCLE SEQUENCERS
    // =========================================================================
    void executeMasterCycle(uint32_t now) {
        if (cycleMode == CYCLE_AUTO_ALL) {
            uint32_t cycleTime = (now - masterCycleStart) % 72000;

            if      (cycleTime < 6000)  currentEmotion = DASAI_IDLE_LOOK;      // Natural look/blink
            else if (cycleTime < 10000) currentEmotion = DASAI_HAPPY_SQUINT;   // Happy smile
            else if (cycleTime < 14000) currentEmotion = DASAI_WINK_L;         // Playful wink
            else if (cycleTime < 18000) currentEmotion = DASAI_SPARKLE_JOY;    // Sparkle joy
            else if (cycleTime < 22000) currentEmotion = DASAI_HEART_LOVE;     // Heart eyes
            else if (cycleTime < 26000) currentEmotion = DASAI_CURIOUS_TILT;   // Curious
            else if (cycleTime < 30000) currentEmotion = DASAI_SURPRISED;      // Surprised
            else if (cycleTime < 34000) currentEmotion = DASAI_EXCITED_BOUNCE; // Dancing party
            else if (cycleTime < 38000) currentEmotion = DASAI_ANGRY_GLARE;    // Angry glare
            else if (cycleTime < 43000) currentEmotion = DASAI_TURBO_RACE;     // Turbo race
            else if (cycleTime < 48000) currentEmotion = DASAI_DRIFT_G_FORCE;  // Drift smoke
            else if (cycleTime < 52500) currentEmotion = DASAI_DIZZY_SPIRAL;   // Dizzy spiral
            else if (cycleTime < 56500) currentEmotion = DASAI_SMUG_CAT;       // Smug cat :3
            else if (cycleTime < 61000) currentEmotion = DASAI_SLEEPY_DROOP;   // Sleepy droop
            else if (cycleTime < 67000) currentEmotion = DASAI_SLEEPING_ZZZ;   // Sleeping Zzz
            else                        currentEmotion = DASAI_SAD_TEAR;       // Sad tears

        } else if (cycleMode == CYCLE_JDM_ACTION) {
            uint32_t cycleTime = (now - masterCycleStart) % 24000;
            if      (cycleTime < 4000)  currentEmotion = DASAI_ANGRY_GLARE;
            else if (cycleTime < 9000)  currentEmotion = DASAI_TURBO_RACE;
            else if (cycleTime < 14000) currentEmotion = DASAI_DRIFT_G_FORCE;
            else if (cycleTime < 18500) currentEmotion = DASAI_DIZZY_SPIRAL;
            else                        currentEmotion = DASAI_SHOCKED_LIGHT;

        } else if (cycleMode == CYCLE_KAWAII_CUTE) {
            uint32_t cycleTime = (now - masterCycleStart) % 24000;
            if      (cycleTime < 4000)  currentEmotion = DASAI_IDLE_LOOK;
            else if (cycleTime < 8000)  currentEmotion = DASAI_HAPPY_SQUINT;
            else if (cycleTime < 12000) currentEmotion = DASAI_SPARKLE_JOY;
            else if (cycleTime < 16500) currentEmotion = DASAI_HEART_LOVE;
            else if (cycleTime < 20500) currentEmotion = DASAI_WINK_L;
            else                        currentEmotion = DASAI_SMUG_CAT;

        } else if (cycleMode == CYCLE_CHILL_RELAX) {
            uint32_t cycleTime = (now - masterCycleStart) % 20000;
            if      (cycleTime < 5000)  currentEmotion = DASAI_IDLE_LOOK;
            else if (cycleTime < 10000) currentEmotion = DASAI_SLEEPY_DROOP;
            else if (cycleTime < 16000) currentEmotion = DASAI_SLEEPING_ZZZ;
            else                        currentEmotion = DASAI_HAPPY_SQUINT;
        }
    }
};
