#pragma once
#include <LovyanGFX.hpp>
#include "display_setup.h"
#include "config.h"
#include "emoji_renderer.h"

// =========================================================================
// DASAI MOCHI COMPREHENSIVE EMOTION ENUM (23 ICONIC EXPRESSIONS & JDM MODES)
// =========================================================================
enum DasaiEmotion {
    DASAI_IDLE_LOOK      = 0,  // 0:  Neutral gaze with organic saccades & blinking
    DASAI_LOOK_LEFT      = 1,  // 1:  Inquisitive side-eye glance left
    DASAI_LOOK_RIGHT     = 2,  // 2:  Inquisitive side-eye glance right
    DASAI_LOOK_UP        = 3,  // 3:  Daydreaming glance up
    DASAI_LOOK_DOWN      = 4,  // 4:  Shy glance down
    DASAI_HAPPY_SQUINT   = 5,  // 5:  Happy upward smile arches (^ ^) + joyful bob
    DASAI_WINK_L         = 6,  // 6:  Left eye playful wink (^ o) + blush
    DASAI_WINK_R         = 7,  // 7:  Right eye playful wink (o ^) + blush
    DASAI_HEART_LOVE     = 8,  // 8:  Dual beating heart eyes (<3 <3) + floating hearts
    DASAI_SPARKLE_JOY    = 9,  // 9:  Anime diamond star/sparkle eyes (✦ ✦)
    DASAI_CURIOUS_TILT   = 10, // 10: Curious head tilt & corner gaze
    DASAI_SURPRISED      = 11, // 11: Startled wide open eyes (O O)
    DASAI_EXCITED_BOUNCE = 12, // 12: Rapid energetic bounce with happy squints
    DASAI_SLEEPY_DROOP   = 13, // 13: Heavy droopy eyelids drifting
    DASAI_SLEEPING_ZZZ   = 14, // 14: Resting slits (- -) + rising Zzz bubbles
    DASAI_ANGRY_GLARE    = 15, // 15: Fierce aggressive JDM brow glare (\ /)
    DASAI_TURBO_RACE     = 16, // 16: Streamlined aerodynamic squish + speed blur lines
    DASAI_DRIFT_G_FORCE  = 17, // 17: High-G drift inertia lean + tire smoke puffs
    DASAI_DIZZY_SPIRAL   = 18, // 18: Hypnotic spinning spirals (@ @) after 360 spin
    DASAI_SHOCKED_LIGHT  = 19, // 19: High-voltage electric lightning bolts (⚡ ⚡)
    DASAI_MATRIX_VISOR   = 20, // 20: JDM Knight-Rider cyber visor laser sweep
    DASAI_SAD_TEAR       = 21, // 21: Sad drooping eyes + falling teardrops
    DASAI_SMUG_CAT       = 22  // 22: Cheeky smug cat eyes (^ w ^) + cute mouth
};

// Master Playback & Choreography Modes
enum DasaiCycleMode {
    CYCLE_AUTO_ALL   = 0, // Master 75s Grand Loop through all 23 emotions
    CYCLE_JDM_ACTION = 1, // Race action: Glare -> Turbo -> Drift -> Dizzy -> Lightning -> Smug
    CYCLE_KAWAII_CUTE= 2, // Kawaii loop: Idle -> Tilt -> Happy -> Sparkle -> Hearts -> Winks -> Smug
    CYCLE_CHILL_RELAX= 3, // Relaxed lounge: Idle -> Saccades -> Sleepy -> Zzz -> Yawn -> Gentle Smile
    CYCLE_MANUAL_LOCK= 4  // Locked to exact chosen emotion from Web Deck
};

// Particle struct for floating hearts, Zzz bubbles, speed streaks, and tears
struct MochiParticle {
    float x;
    float y;
    float vx;
    float vy;
    float size;
    float alpha;
    uint8_t type; // 0=heart, 1=zzz, 2=speedline, 3=teardrop, 4=smoke, 5=sparkle
    bool active;
};

class DasaiMochi {
public:
    DasaiEmotion  currentEmotion = DASAI_IDLE_LOOK;
    DasaiCycleMode cycleMode     = CYCLE_AUTO_ALL;
    int           currentStyle   = 0; // 0=Warm White, 1=Pastel Cyan, 2=Sakura Pink, 3=Mochi Gold, 4=Cyber Violet
    
    // 2-Second Hold Secret Reaction Card
    bool     isShyLoveActive   = false;
    uint32_t shyLoveStartTime  = 0;
    DasaiEmotion preShyEmotion = DASAI_IDLE_LOOK;
    String   customShyText     = "I LOVE YOU :heart: :sparkles:";

private:
    // Base Eye Geometry
    float baseEyeWidth  = 42.0f;
    float baseEyeHeight = 66.0f;
    float baseRadius    = 18.0f;
    float eyeSpacing    = 32.0f;

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

    // Eye shape / eyelid factors (-1.0=sleepy slit, 0.0=pill, 1.0=happy arch, 2.0=angry slant)
    float curLeftArc = 0.0f;
    float targetLeftArc = 0.0f;
    float curRightArc = 0.0f;
    float targetRightArc = 0.0f;

    // Head tilt angle in degrees
    float curTiltAngle = 0.0f;
    float targetTiltAngle = 0.0f;

    // State Timers & Sequencer
    uint32_t stateStartTime   = 0;
    uint32_t lastGazeChange   = 0;
    uint32_t lastBlinkTime    = 0;
    bool     isBlinking       = false;
    uint32_t masterCycleStart = 0;
    int      animPhase        = 0;

    // Particle System Buffer (Fixed zero-heap allocation)
    static const int MAX_PARTICLES = 16;
    MochiParticle particles[MAX_PARTICLES];
    uint32_t lastParticleSpawn = 0;

public:
    DasaiMochi() {
        masterCycleStart = millis();
        stateStartTime = millis();
        for (int i = 0; i < MAX_PARTICLES; i++) {
            particles[i].active = false;
        }
    }

    void setStyle(int style) {
        currentStyle = (style >= 0 && style < 5) ? style : 0;
    }

    int cycleStyle() {
        currentStyle = (currentStyle + 1) % 5;
        return currentStyle;
    }

    void setEmotion(DasaiEmotion emo) {
        currentEmotion = emo;
        cycleMode = CYCLE_MANUAL_LOCK;
        stateStartTime = millis();
        resetParticles();
    }

    void setCycleMode(DasaiCycleMode mode) {
        cycleMode = mode;
        masterCycleStart = millis();
        stateStartTime = millis();
        resetParticles();
    }

    uint16_t getEyeColor() {
        if (isShyLoveActive || currentEmotion == DASAI_HEART_LOVE) return 0xF81F; // Vivid Pink Love
        switch (currentStyle) {
            case 0: return 0xFFFF; // Warm Crisp White / OLED Pearl
            case 1: return 0x7FFF; // Pastel Kawaii Cyan
            case 2: return 0xFDF7; // Sakura Pink
            case 3: return 0xFFE0; // Mochi Gold / JDM Amber
            case 4: return 0xD69A; // Cyber Violet / Spearhead
            default: return 0xFFFF;
        }
    }

    void setShyText(const String& txt) {
        if (txt.length() > 0) customShyText = txt;
    }

    void triggerShyLove(uint32_t durMs = 5000) {
        if (!isShyLoveActive) preShyEmotion = currentEmotion;
        isShyLoveActive = true;
        shyLoveStartTime = millis();
        currentEmotion = DASAI_HEART_LOVE;
        targetCenterY = 92.0f; // Move up slightly for bottom card
        spawnParticles(0, 8);
    }

    void triggerTap() {
        // Playful reaction: switches to excited bounce & giggle
        currentEmotion = (currentEmotion == DASAI_HAPPY_SQUINT) ? DASAI_EXCITED_BOUNCE : DASAI_HAPPY_SQUINT;
        stateStartTime = millis();
        targetCenterY = 100.0f;
    }

    void triggerTripleTapTurbo() {
        // High-octane triple-tap boost!
        currentEmotion = DASAI_TURBO_RACE;
        stateStartTime = millis();
        spawnParticles(2, 6);
    }

    void update() {
        uint32_t now = millis();
        animPhase = (animPhase + 4) % 3600;

        // Auto revert from shy love after 5 seconds
        if (isShyLoveActive && (now - shyLoveStartTime >= 5000)) {
            isShyLoveActive = false;
            currentEmotion = preShyEmotion;
            targetCenterY = 110.0f;
        }

        // =====================================================================
        // CHOREOGRAPHED CYCLE SEQUENCERS
        // =====================================================================
        if (!isShyLoveActive && cycleMode != CYCLE_MANUAL_LOCK) {
            executeMasterCycle(now);
        }

        // =====================================================================
        // APPLY EMOTION GEOMETRY & TARGET PARAMETERS
        // =====================================================================
        applyEmotionGeometry(now);

        // =====================================================================
        // NATURAL SACCADES & ORGANIC BLINK ENGINE
        // =====================================================================
        updateBlinkAndGaze(now);

        // =====================================================================
        // PHYSICS EASING & EXPONENTIAL INTERPOLATION
        // =====================================================================
        float breathY = (currentEmotion == DASAI_SLEEPING_ZZZ || currentEmotion == DASAI_SLEEPY_DROOP)
                        ? sin(now * 0.0018f) * 3.5f
                        : sin(now * 0.003f) * 2.0f;

        curCenterX   += (targetCenterX - curCenterX) * 0.24f;
        curCenterY   += ((targetCenterY + breathY) - curCenterY) * 0.24f;
        curLeftH     += (targetLeftH - curLeftH) * 0.35f;
        curRightH    += (targetRightH - curRightH) * 0.35f;
        curLeftW     += (targetLeftW - curLeftW) * 0.26f;
        curRightW    += (targetRightW - curRightW) * 0.26f;
        curLeftArc   += (targetLeftArc - curLeftArc) * 0.32f;
        curRightArc  += (targetRightArc - curRightArc) * 0.32f;
        curTiltAngle += (targetTiltAngle - curTiltAngle) * 0.20f;

        // =====================================================================
        // RENDER DASAI MOCHI SCENE TO CANVAS
        // =====================================================================
        canvas.fillScreen(TFT_BLACK);

        uint16_t eyeColor = getEyeColor();
        float leftX = curCenterX - (curLeftW / 2.0f) - (eyeSpacing / 2.0f);
        float rightX = curCenterX + (curRightW / 2.0f) + (eyeSpacing / 2.0f);

        // 1. Render Special Full-Screen Visor Laser Mode
        if (currentEmotion == DASAI_MATRIX_VISOR) {
            renderMatrixVisor(eyeColor);
            return;
        }

        // 2. Render Background Particles (Speed lines, Drift smoke, Zzz, Sparkles, Hearts)
        updateAndRenderParticles(eyeColor);

        // 3. Render Left Eye Unit
        renderEye(leftX, curCenterY, curLeftW, curLeftH, curLeftArc, eyeColor, true);

        // 4. Render Right Eye Unit
        renderEye(rightX, curCenterY, curRightW, curRightH, curRightArc, eyeColor, false);

        // 5. Render Kawaii Blushing Cheeks
        if (isShyLoveActive || currentEmotion == DASAI_HAPPY_SQUINT || currentEmotion == DASAI_WINK_L ||
            currentEmotion == DASAI_WINK_R || currentEmotion == DASAI_HEART_LOVE || currentEmotion == DASAI_SPARKLE_JOY ||
            currentEmotion == DASAI_EXCITED_BOUNCE || currentEmotion == DASAI_SMUG_CAT) {
            renderBlush(leftX, rightX, curCenterY);
        }

        // 6. Render Smug Cat Smile Mouth
        if (currentEmotion == DASAI_SMUG_CAT) {
            renderSmugMouth(curCenterX, curCenterY + 36.0f, eyeColor);
        }

        // 7. Render Sad Teardrops
        if (currentEmotion == DASAI_SAD_TEAR) {
            renderSadTeardrops(leftX, rightX, curCenterY);
        }

        // 8. Render 2-Second Hold Secret Message Overlay
        if (isShyLoveActive) {
            renderShyLoveCard();
        }
    }

private:
    // -------------------------------------------------------------------------
    // MASTER SEQUENCER ENGINE
    // -------------------------------------------------------------------------
    void executeMasterCycle(uint32_t now) {
        if (cycleMode == CYCLE_AUTO_ALL) {
            // Full 72-second master story loop showcasing all 23 expressions
            uint32_t cycleTime = (now - masterCycleStart) % 72000;

            if      (cycleTime < 4000)  currentEmotion = DASAI_IDLE_LOOK;
            else if (cycleTime < 7500)  currentEmotion = DASAI_HAPPY_SQUINT;
            else if (cycleTime < 11000) currentEmotion = DASAI_WINK_L;
            else if (cycleTime < 14500) currentEmotion = DASAI_CURIOUS_TILT;
            else if (cycleTime < 18000) currentEmotion = DASAI_SPARKLE_JOY;
            else if (cycleTime < 22000) currentEmotion = DASAI_HEART_LOVE;
            else if (cycleTime < 25500) currentEmotion = DASAI_WINK_R;
            else if (cycleTime < 29000) currentEmotion = DASAI_SURPRISED;
            else if (cycleTime < 33000) currentEmotion = DASAI_EXCITED_BOUNCE;
            else if (cycleTime < 37000) currentEmotion = DASAI_ANGRY_GLARE;
            else if (cycleTime < 41500) currentEmotion = DASAI_TURBO_RACE;
            else if (cycleTime < 46000) currentEmotion = DASAI_DRIFT_G_FORCE;
            else if (cycleTime < 50000) currentEmotion = DASAI_DIZZY_SPIRAL;
            else if (cycleTime < 53500) currentEmotion = DASAI_SHOCKED_LIGHT;
            else if (cycleTime < 57000) currentEmotion = DASAI_SMUG_CAT;
            else if (cycleTime < 61000) currentEmotion = DASAI_SLEEPY_DROOP;
            else if (cycleTime < 66500) currentEmotion = DASAI_SLEEPING_ZZZ;
            else if (cycleTime < 69500) currentEmotion = DASAI_SAD_TEAR;
            else                        currentEmotion = DASAI_MATRIX_VISOR;

        } else if (cycleMode == CYCLE_JDM_ACTION) {
            // 24s High-Octane Racing Action Loop
            uint32_t cycleTime = (now - masterCycleStart) % 24000;
            if      (cycleTime < 3500)  currentEmotion = DASAI_ANGRY_GLARE;
            else if (cycleTime < 7500)  currentEmotion = DASAI_TURBO_RACE;
            else if (cycleTime < 12000) currentEmotion = DASAI_DRIFT_G_FORCE;
            else if (cycleTime < 15500) currentEmotion = DASAI_DIZZY_SPIRAL;
            else if (cycleTime < 19000) currentEmotion = DASAI_SHOCKED_LIGHT;
            else                        currentEmotion = DASAI_SMUG_CAT;

        } else if (cycleMode == CYCLE_KAWAII_CUTE) {
            // 24s Kawaii Cute & Affection Loop
            uint32_t cycleTime = (now - masterCycleStart) % 24000;
            if      (cycleTime < 3500)  currentEmotion = DASAI_IDLE_LOOK;
            else if (cycleTime < 7000)  currentEmotion = DASAI_CURIOUS_TILT;
            else if (cycleTime < 11000) currentEmotion = DASAI_HAPPY_SQUINT;
            else if (cycleTime < 15000) currentEmotion = DASAI_SPARKLE_JOY;
            else if (cycleTime < 19500) currentEmotion = DASAI_HEART_LOVE;
            else                        currentEmotion = DASAI_WINK_L;

        } else if (cycleMode == CYCLE_CHILL_RELAX) {
            // 20s Relaxed & Sleepy Loop
            uint32_t cycleTime = (now - masterCycleStart) % 20000;
            if      (cycleTime < 4500)  currentEmotion = DASAI_IDLE_LOOK;
            else if (cycleTime < 8500)  currentEmotion = DASAI_SLEEPY_DROOP;
            else if (cycleTime < 15000) currentEmotion = DASAI_SLEEPING_ZZZ;
            else                        currentEmotion = DASAI_HAPPY_SQUINT;
        }
    }

    // -------------------------------------------------------------------------
    // EMOTION GEOMETRY TARGET CALCULATOR
    // -------------------------------------------------------------------------
    void applyEmotionGeometry(uint32_t now) {
        switch (currentEmotion) {
            case DASAI_IDLE_LOOK:
                targetLeftH = baseEyeHeight;  targetRightH = baseEyeHeight;
                targetLeftW = baseEyeWidth;   targetRightW = baseEyeWidth;
                targetLeftArc = 0.0f;         targetRightArc = 0.0f;
                targetTiltAngle = 0.0f;
                break;

            case DASAI_LOOK_LEFT:
                targetCenterX = 98.0f;        targetCenterY = 110.0f;
                targetLeftH = baseEyeHeight;  targetRightH = baseEyeHeight;
                targetLeftW = baseEyeWidth;   targetRightW = baseEyeWidth;
                targetLeftArc = 0.0f;         targetRightArc = 0.0f;
                targetTiltAngle = 0.0f;
                break;

            case DASAI_LOOK_RIGHT:
                targetCenterX = 142.0f;       targetCenterY = 110.0f;
                targetLeftH = baseEyeHeight;  targetRightH = baseEyeHeight;
                targetLeftW = baseEyeWidth;   targetRightW = baseEyeWidth;
                targetLeftArc = 0.0f;         targetRightArc = 0.0f;
                targetTiltAngle = 0.0f;
                break;

            case DASAI_LOOK_UP:
                targetCenterX = 120.0f;       targetCenterY = 96.0f;
                targetLeftH = baseEyeHeight;  targetRightH = baseEyeHeight;
                targetLeftW = baseEyeWidth;   targetRightW = baseEyeWidth;
                targetLeftArc = 0.0f;         targetRightArc = 0.0f;
                targetTiltAngle = 0.0f;
                break;

            case DASAI_LOOK_DOWN:
                targetCenterX = 120.0f;       targetCenterY = 124.0f;
                targetLeftH = 48.0f;          targetRightH = 48.0f;
                targetLeftW = baseEyeWidth;   targetRightW = baseEyeWidth;
                targetLeftArc = 0.0f;         targetRightArc = 0.0f;
                targetTiltAngle = 0.0f;
                break;

            case DASAI_HAPPY_SQUINT: {
                targetCenterX = 120.0f;
                float bounce = sin(now * 0.009f) * 6.0f;
                targetCenterY = 110.0f - abs((int)bounce);
                targetLeftH = 36.0f;          targetRightH = 36.0f;
                targetLeftW = 46.0f;          targetRightW = 46.0f;
                targetLeftArc = 1.0f;         targetRightArc = 1.0f; // Happy smile arc
                targetTiltAngle = 0.0f;
                break;
            }

            case DASAI_WINK_L:
                targetCenterX = 122.0f;       targetCenterY = 110.0f;
                targetLeftH = 34.0f;          targetRightH = 68.0f;
                targetLeftW = 46.0f;          targetRightW = 44.0f;
                targetLeftArc = 1.0f;         targetRightArc = 0.0f;
                targetTiltAngle = -4.0f;
                break;

            case DASAI_WINK_R:
                targetCenterX = 118.0f;       targetCenterY = 110.0f;
                targetLeftH = 68.0f;          targetRightH = 34.0f;
                targetLeftW = 44.0f;          targetRightW = 46.0f;
                targetLeftArc = 0.0f;         targetRightArc = 1.0f;
                targetTiltAngle = 4.0f;
                break;

            case DASAI_HEART_LOVE: {
                targetCenterX = 120.0f;       targetCenterY = isShyLoveActive ? 92.0f : 110.0f;
                float beat = sin(now * 0.012f) * 4.0f;
                targetLeftH = 58.0f + beat;   targetRightH = 58.0f + beat;
                targetLeftW = 48.0f + beat;   targetRightW = 48.0f + beat;
                targetLeftArc = 0.0f;         targetRightArc = 0.0f;
                targetTiltAngle = 0.0f;
                spawnParticles(0, 1); // Emit floating hearts
                break;
            }

            case DASAI_SPARKLE_JOY: {
                targetCenterX = 120.0f;       targetCenterY = 108.0f;
                float pulse = sin(now * 0.010f) * 3.0f;
                targetLeftH = 64.0f + pulse;  targetRightH = 64.0f + pulse;
                targetLeftW = 48.0f + pulse;  targetRightW = 48.0f + pulse;
                targetLeftArc = 0.0f;         targetRightArc = 0.0f;
                targetTiltAngle = 0.0f;
                spawnParticles(5, 1); // Emit sparkle glints
                break;
            }

            case DASAI_CURIOUS_TILT:
                targetCenterX = 134.0f;       targetCenterY = 102.0f;
                targetLeftH = 64.0f;          targetRightH = 64.0f;
                targetLeftW = 42.0f;          targetRightW = 42.0f;
                targetLeftArc = 0.0f;         targetRightArc = 0.0f;
                targetTiltAngle = 14.0f;      // Inquisitive head tilt
                break;

            case DASAI_SURPRISED:
                targetCenterX = 120.0f;       targetCenterY = 106.0f;
                targetLeftH = 72.0f;          targetRightH = 72.0f;
                targetLeftW = 54.0f;          targetRightW = 54.0f;
                targetLeftArc = 0.0f;         targetRightArc = 0.0f;
                targetTiltAngle = 0.0f;
                break;

            case DASAI_EXCITED_BOUNCE: {
                targetCenterX = 120.0f;
                float fastBounce = sin(now * 0.020f) * 9.0f;
                targetCenterY = 108.0f - abs((int)fastBounce);
                targetLeftH = 34.0f;          targetRightH = 34.0f;
                targetLeftW = 48.0f;          targetRightW = 48.0f;
                targetLeftArc = 1.0f;         targetRightArc = 1.0f;
                targetTiltAngle = sin(now * 0.015f) * 5.0f;
                break;
            }

            case DASAI_SLEEPY_DROOP:
                targetCenterX = 120.0f;       targetCenterY = 114.0f;
                targetLeftH = 22.0f;          targetRightH = 22.0f;
                targetLeftW = 44.0f;          targetRightW = 44.0f;
                targetLeftArc = -0.5f;        targetRightArc = -0.5f;
                targetTiltAngle = 0.0f;
                break;

            case DASAI_SLEEPING_ZZZ:
                targetCenterX = 120.0f;       targetCenterY = 116.0f;
                targetLeftH = 8.0f;           targetRightH = 8.0f;
                targetLeftW = 46.0f;          targetRightW = 46.0f;
                targetLeftArc = -1.0f;        targetRightArc = -1.0f; // Sleep slit
                targetTiltAngle = 0.0f;
                spawnParticles(1, 1); // Emit Zzz bubbles
                break;

            case DASAI_ANGRY_GLARE:
                targetCenterX = 120.0f;       targetCenterY = 112.0f;
                targetLeftH = 44.0f;          targetRightH = 44.0f;
                targetLeftW = 46.0f;          targetRightW = 46.0f;
                targetLeftArc = 2.0f;         targetRightArc = 2.0f; // Slanted angry brows
                targetTiltAngle = 0.0f;
                break;

            case DASAI_TURBO_RACE: {
                targetCenterX = 112.0f;       targetCenterY = 112.0f;
                // Aerodynamic squish
                targetLeftH = 38.0f;          targetRightH = 38.0f;
                targetLeftW = 54.0f;          targetRightW = 54.0f;
                targetLeftArc = 2.0f;         targetRightArc = 2.0f;
                targetTiltAngle = -8.0f;
                spawnParticles(2, 2); // Emit high-speed streak lines
                break;
            }

            case DASAI_DRIFT_G_FORCE: {
                // High-G drift inertia sway
                float gSway = sin(now * 0.006f) * 16.0f;
                targetCenterX = 120.0f + gSway;
                targetCenterY = 112.0f;
                targetLeftH = 50.0f;          targetRightH = 50.0f;
                targetLeftW = 44.0f;          targetRightW = 44.0f;
                targetLeftArc = 2.0f;         targetRightArc = 2.0f;
                targetTiltAngle = (gSway > 0) ? 12.0f : -12.0f;
                spawnParticles(4, 2); // Drift tire smoke
                break;
            }

            case DASAI_DIZZY_SPIRAL:
                targetCenterX = 120.0f;       targetCenterY = 110.0f;
                targetLeftH = 62.0f;          targetRightH = 62.0f;
                targetLeftW = 50.0f;          targetRightW = 50.0f;
                targetLeftArc = 0.0f;         targetRightArc = 0.0f;
                targetTiltAngle = sin(now * 0.008f) * 8.0f;
                break;

            case DASAI_SHOCKED_LIGHT:
                targetCenterX = 120.0f;       targetCenterY = 108.0f;
                targetLeftH = 68.0f;          targetRightH = 68.0f;
                targetLeftW = 50.0f;          targetRightW = 50.0f;
                targetLeftArc = 0.0f;         targetRightArc = 0.0f;
                targetTiltAngle = 0.0f;
                break;

            case DASAI_SAD_TEAR:
                targetCenterX = 120.0f;       targetCenterY = 114.0f;
                targetLeftH = 46.0f;          targetRightH = 46.0f;
                targetLeftW = 42.0f;          targetRightW = 42.0f;
                targetLeftArc = -0.5f;        targetRightArc = -0.5f;
                targetTiltAngle = 0.0f;
                spawnParticles(3, 1); // Falling teardrops
                break;

            case DASAI_SMUG_CAT:
                targetCenterX = 120.0f;       targetCenterY = 108.0f;
                targetLeftH = 32.0f;          targetRightH = 32.0f;
                targetLeftW = 48.0f;          targetRightW = 48.0f;
                targetLeftArc = 1.0f;         targetRightArc = 1.0f;
                targetTiltAngle = 0.0f;
                break;

            default:
                break;
        }
    }

    // -------------------------------------------------------------------------
    // BLINK & SACCADE GAZE ENGINE
    // -------------------------------------------------------------------------
    void updateBlinkAndGaze(uint32_t now) {
        if (currentEmotion == DASAI_IDLE_LOOK || currentEmotion == DASAI_CURIOUS_TILT) {
            // Saccade gaze shifting
            if (now - lastGazeChange > 2200 && !isBlinking) {
                lastGazeChange = now;
                int r = random(0, 5);
                if (r == 0)      { targetCenterX = 120.0f; targetCenterY = 110.0f; }
                else if (r == 1) { targetCenterX = 104.0f; targetCenterY = 108.0f; } // look left
                else if (r == 2) { targetCenterX = 136.0f; targetCenterY = 108.0f; } // look right
                else if (r == 3) { targetCenterX = 120.0f; targetCenterY = 100.0f; } // look up
                else             { targetCenterX = 120.0f; targetCenterY = 116.0f; } // look down
            }

            // Natural Blink
            if (now - lastBlinkTime > 3600 && !isBlinking) {
                isBlinking = true;
                lastBlinkTime = now;
            } else if (isBlinking) {
                uint32_t blinkProgress = now - lastBlinkTime;
                if (blinkProgress < 75) {
                    targetLeftH = 4.0f;
                    targetRightH = 4.0f;
                } else if (blinkProgress < 160) {
                    targetLeftH = baseEyeHeight + 4.0f; // Elastic pop overshoot
                    targetRightH = baseEyeHeight + 4.0f;
                } else {
                    isBlinking = false;
                    targetLeftH = baseEyeHeight;
                    targetRightH = baseEyeHeight;
                }
            }
        }
    }

    // -------------------------------------------------------------------------
    // EYE UNIT VECTOR RENDERER (SHAPES, ARCS, HEARTS, STARS, SPIRALS, LIGHTNING)
    // -------------------------------------------------------------------------
    void renderEye(float cx, float cy, float w, float h, float arcVal, uint16_t col, bool isLeft) {
        // 1. Heart Eye Shape
        if (currentEmotion == DASAI_HEART_LOVE || isShyLoveActive) {
            drawVectorHeart((int)cx, (int)cy, (int)(w * 0.52f), col);
            return;
        }

        // 2. Anime Diamond Star / Sparkle Eye Shape
        if (currentEmotion == DASAI_SPARKLE_JOY) {
            drawSparkleStar((int)cx, (int)cy, (int)(w * 0.55f), (int)(h * 0.55f), col);
            return;
        }

        // 3. Hypnotic Spiral Eye Shape (Dizzy Mode)
        if (currentEmotion == DASAI_DIZZY_SPIRAL) {
            drawDizzySpiral((int)cx, (int)cy, (int)(w * 0.50f), col);
            return;
        }

        // 4. Shocked Electric Lightning Bolt Eye Shape
        if (currentEmotion == DASAI_SHOCKED_LIGHT) {
            drawLightningBolt((int)cx, (int)cy, (int)w, (int)h, col, isLeft);
            return;
        }

        // 5. Happy Smile Upward Arch (^)
        if (arcVal > 0.4f && currentEmotion != DASAI_ANGRY_GLARE && currentEmotion != DASAI_TURBO_RACE && currentEmotion != DASAI_DRIFT_G_FORCE) {
            drawHappyArch((int)cx, (int)cy, (int)w, col);
            return;
        }

        // 6. Sleeping / Blinking Slit (-)
        if (arcVal < -0.4f || h <= 8.0f) {
            canvas.fillRoundRect((int)(cx - (w / 2.0f)), (int)(cy - 3.0f), (int)w, 6, 3, col);
            return;
        }

        // 7. Angry / JDM Bosozoku Slanted Glare (\ /)
        if (arcVal >= 1.5f || currentEmotion == DASAI_ANGRY_GLARE || currentEmotion == DASAI_TURBO_RACE || currentEmotion == DASAI_DRIFT_G_FORCE) {
            drawAngryEye((int)cx, (int)cy, (int)w, (int)h, col, isLeft);
            return;
        }

        // 8. Standard Expressive Rounded Capsule
        int rx = (int)(cx - (w / 2.0f));
        int ry = (int)(cy - (h / 2.0f));
        int rad = (int)baseRadius;
        if (rad > (int)(w / 2)) rad = (int)(w / 2);
        if (rad > (int)(h / 2)) rad = (int)(h / 2);

        canvas.fillRoundRect(rx, ry, (int)w, (int)h, rad, col);

        // Specular eye highlight dot in top-left
        if (h > 30.0f) {
            canvas.fillCircle((int)(cx - (w * 0.22f)), (int)(cy - (h * 0.25f)), 4, TFT_WHITE);
        }
    }

    // -------------------------------------------------------------------------
    // VECTOR DRAWING PRIMITIVES
    // -------------------------------------------------------------------------
    void drawHappyArch(int cx, int cy, int w, uint16_t col) {
        int r = w / 2;
        int topY = cy - 6;
        for (int thick = 0; thick < 7; thick++) {
            canvas.drawCircle(cx, topY + 12, r - thick, col);
        }
        // Mask lower half so only upward rainbow arch is visible
        canvas.fillRect(cx - w - 2, topY + 12, w * 2 + 4, w, TFT_BLACK);
    }

    void drawAngryEye(int cx, int cy, int w, int h, uint16_t col, bool isLeft) {
        int rx = cx - (w / 2);
        int ry = cy - (h / 2);
        canvas.fillRoundRect(rx, ry, w, h, 8, col);

        // Slanted eyebrow cutter triangle (Left tilts down to right, Right tilts down to left)
        if (isLeft) {
            // Cut top-right corner
            canvas.fillTriangle(rx + w + 2, ry - 2, rx + w + 2, ry + (h / 2) + 2, rx + (w / 3), ry - 2, TFT_BLACK);
        } else {
            // Cut top-left corner
            canvas.fillTriangle(rx - 2, ry - 2, rx - 2, ry + (h / 2) + 2, rx + (w * 2 / 3), ry - 2, TFT_BLACK);
        }

        // Intense small glowing pupil dot
        canvas.fillCircle(cx, cy + 4, 3, TFT_WHITE);
    }

    void drawVectorHeart(int cx, int cy, int sz, uint16_t col) {
        int r = sz / 2;
        canvas.fillCircle(cx - (sz / 2), cy - (sz / 4), r, col);
        canvas.fillCircle(cx + (sz / 2), cy - (sz / 4), r, col);
        canvas.fillTriangle(cx - sz - 1, cy - (sz / 4), cx + sz + 1, cy - (sz / 4), cx, cy + sz, col);

        // Specular gleam in top-left lobe
        canvas.fillCircle(cx - (sz / 2) - 2, cy - (sz / 4) - 2, 2, TFT_WHITE);
    }

    void drawSparkleStar(int cx, int cy, int rx, int ry, uint16_t col) {
        // 4-point anime diamond sparkle star
        canvas.fillTriangle(cx, cy - ry, cx - (rx / 3), cy, cx + (rx / 3), cy, col);
        canvas.fillTriangle(cx, cy + ry, cx - (rx / 3), cy, cx + (rx / 3), cy, col);
        canvas.fillTriangle(cx - rx, cy, cx, cy - (ry / 3), cx, cy + (ry / 3), col);
        canvas.fillTriangle(cx + rx, cy, cx, cy - (ry / 3), cx, cy + (ry / 3), col);

        // Core bright glow
        canvas.fillCircle(cx, cy, 3, TFT_WHITE);
    }

    void drawDizzySpiral(int cx, int cy, int maxR, uint16_t col) {
        float rot = (animPhase * 0.06f);
        for (int r = 4; r <= maxR; r += 5) {
            for (float a = 0; a < 6.28f; a += 0.4f) {
                float curA = a + rot + (r * 0.15f);
                int px = cx + (int)(cos(curA) * r);
                int py = cy + (int)(sin(curA) * r);
                canvas.drawPixel(px, py, col);
                canvas.drawPixel(px + 1, py, col);
            }
        }
    }

    void drawLightningBolt(int cx, int cy, int w, int h, uint16_t col, bool isLeft) {
        int topX = cx + (isLeft ? 4 : -4);
        int topY = cy - (h / 2);
        int midX1 = cx - (isLeft ? 8 : -8);
        int midY1 = cy;
        int midX2 = cx + (isLeft ? 2 : -2);
        int midY2 = cy - 2;
        int botX = cx - (isLeft ? 6 : -6);
        int botY = cy + (h / 2);

        canvas.fillTriangle(topX, topY, midX1, midY1, midX2, midY2, col);
        canvas.fillTriangle(midX1, midY1, midX2, midY2, botX, botY, col);

        // Bright electric core
        canvas.drawLine(topX, topY, botX, botY, TFT_WHITE);
    }

    void renderSmugMouth(float cx, float cy, uint16_t col) {
        // Cute cat "w" mouth
        int my = (int)cy;
        int mx = (int)cx;
        canvas.drawCircle(mx - 7, my, 6, col);
        canvas.drawCircle(mx + 7, my, 6, col);
        canvas.fillRect(mx - 15, my - 8, 30, 8, TFT_BLACK); // Top mask
    }

    void renderSadTeardrops(float lx, float rx, float cy) {
        uint16_t tearCol = 0x7FFF; // Water cyan
        int phase = (animPhase * 2) % 120;
        int ty = (int)cy + 24 + (phase / 2);
        canvas.fillCircle((int)lx + 8, ty, 3, tearCol);
        canvas.fillTriangle((int)lx + 6, ty, (int)lx + 10, ty, (int)lx + 8, ty - 5, tearCol);

        canvas.fillCircle((int)rx - 8, ty + 6, 3, tearCol);
        canvas.fillTriangle((int)rx - 10, ty + 6, (int)rx - 6, ty + 6, (int)rx - 8, ty + 1, tearCol);
    }

    void renderBlush(float lx, float rx, float cy) {
        float blushY = cy + 36.0f;
        uint16_t blushCol = 0xF9AE; // Soft Kawaii Sakura Pink
        canvas.fillRoundRect((int)(lx - 14), (int)blushY, 28, 10, 5, blushCol);
        canvas.fillRoundRect((int)(rx - 14), (int)blushY, 28, 10, 5, blushCol);
    }

    void renderMatrixVisor(uint16_t col) {
        // Full screen JDM Knight-Rider / Bosozoku Cyber Laser Visor
        canvas.drawRoundRect(14, 104, 212, 32, 4, 0x2104);
        canvas.fillRect(16, 106, 208, 28, 0x0821);

        float scanPos = (sin(animPhase * 0.05f) + 1.0f) * 0.5f; // 0.0 to 1.0
        int beamX = 20 + (int)(scanPos * 160.0f);

        // Phosphor trailing fade
        for (int i = 0; i < 38; i++) {
            int bx = beamX - 19 + i;
            if (bx >= 18 && bx <= 222) {
                float dist = abs(i - 19) / 19.0f;
                uint16_t bCol = (dist < 0.25f) ? TFT_WHITE : col;
                canvas.drawFastVLine(bx, 108, 24, bCol);
            }
        }

        // Visor top/bottom tick marks
        for (int x = 20; x <= 220; x += 20) {
            canvas.drawFastVLine(x, 98, 4, 0x4208);
            canvas.drawFastVLine(x, 138, 4, 0x4208);
        }

        canvas.setTextColor(col, TFT_BLACK);
        canvas.setTextSize(1);
        canvas.drawCenterString("JDM VISOR // ACTIVE SCAN", 120, 162);
    }

    // -------------------------------------------------------------------------
    // PARTICLE ENGINE (HEARTS, ZZZ, SPEED LINES, SMOKE, SPARKLES)
    // -------------------------------------------------------------------------
    void resetParticles() {
        for (int i = 0; i < MAX_PARTICLES; i++) {
            particles[i].active = false;
        }
    }

    void spawnParticles(uint8_t type, int count) {
        uint32_t now = millis();
        if (now - lastParticleSpawn < 180) return;
        lastParticleSpawn = now;

        for (int c = 0; c < count; c++) {
            for (int i = 0; i < MAX_PARTICLES; i++) {
                if (!particles[i].active) {
                    particles[i].active = true;
                    particles[i].type = type;
                    particles[i].alpha = 1.0f;

                    if (type == 0) { // Heart
                        particles[i].x = random(30, 210);
                        particles[i].y = 130 + random(0, 40);
                        particles[i].vx = (random(-10, 10)) / 15.0f;
                        particles[i].vy = -(random(15, 30)) / 10.0f;
                        particles[i].size = random(4, 7);
                    } else if (type == 1) { // Zzz
                        particles[i].x = 150 + random(0, 20);
                        particles[i].y = 95;
                        particles[i].vx = (random(5, 15)) / 10.0f;
                        particles[i].vy = -(random(8, 16)) / 10.0f;
                        particles[i].size = random(1, 3);
                    } else if (type == 2) { // Speedline
                        particles[i].x = 240;
                        particles[i].y = random(80, 140);
                        particles[i].vx = -(random(18, 32));
                        particles[i].vy = 0;
                        particles[i].size = random(16, 48);
                    } else if (type == 4) { // Drift smoke
                        particles[i].x = random(40, 200);
                        particles[i].y = 150;
                        particles[i].vx = (random(-15, 15)) / 10.0f;
                        particles[i].vy = -(random(6, 14)) / 10.0f;
                        particles[i].size = random(3, 8);
                    } else if (type == 5) { // Sparkles
                        particles[i].x = random(20, 220);
                        particles[i].y = random(30, 190);
                        particles[i].vx = 0;
                        particles[i].vy = -0.5f;
                        particles[i].size = random(3, 6);
                    }
                    break;
                }
            }
        }
    }

    void updateAndRenderParticles(uint16_t col) {
        for (int i = 0; i < MAX_PARTICLES; i++) {
            if (!particles[i].active) continue;

            particles[i].x += particles[i].vx;
            particles[i].y += particles[i].vy;

            // Render Particle by Type
            if (particles[i].type == 0) {
                // Floating Mini Heart
                drawVectorHeart((int)particles[i].x, (int)particles[i].y, (int)particles[i].size, 0xF81F);
                if (particles[i].y < 20) particles[i].active = false;

            } else if (particles[i].type == 1) {
                // Rising Zzz
                canvas.setTextColor(0x7FFF, TFT_BLACK);
                canvas.setTextSize((int)particles[i].size);
                canvas.drawString("z", (int)particles[i].x, (int)particles[i].y);
                if (particles[i].y < 20 || particles[i].x > 230) particles[i].active = false;

            } else if (particles[i].type == 2) {
                // Turbo horizontal speed streak line
                canvas.drawFastHLine((int)particles[i].x, (int)particles[i].y, (int)particles[i].size, col);
                if (particles[i].x + particles[i].size < 0) particles[i].active = false;

            } else if (particles[i].type == 4) {
                // Drift tire smoke puff
                canvas.drawCircle((int)particles[i].x, (int)particles[i].y, (int)particles[i].size, 0x52AA);
                if (particles[i].y < 60) particles[i].active = false;

            } else if (particles[i].type == 5) {
                // Sparkle glint
                drawSparkleStar((int)particles[i].x, (int)particles[i].y, (int)particles[i].size, (int)particles[i].size, TFT_WHITE);
                if (particles[i].y < 10) particles[i].active = false;
            }
        }
    }

    // -------------------------------------------------------------------------
    // 2-SECOND HOLD SECRET CARD OVERLAY
    // -------------------------------------------------------------------------
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
