#pragma once
#include <Arduino.h>
#include <LovyanGFX.hpp>
#include "display_setup.h"
#include "config.h"
#include "emoji_renderer.h"
#include "dasai_gif_player.h"

extern LGFX_Sprite canvas;

// =========================================================================
// DASAI MOCHI AUTHENTIC EMOTIONS (43 ORIGINAL GIF FILES)
// =========================================================================
enum DasaiEmotion {
    DASAI_IDLE_LOOK      = 0,  // blank.gif
    DASAI_LOOK_LEFT      = 1,  // left.gif
    DASAI_LOOK_RIGHT     = 2,  // right.gif
    DASAI_LOOK_UP        = 3,  // serene.gif
    DASAI_LOOK_DOWN      = 4,  // down.gif
    DASAI_HAPPY_SQUINT   = 5,  // happy.gif
    DASAI_WINK_L         = 6,  // wink.gif
    DASAI_WINK_R         = 7,  // wink.gif
    DASAI_HEART_LOVE     = 8,  // love.gif
    DASAI_SPARKLE_JOY    = 9,  // sparkle.gif
    DASAI_CURIOUS_TILT   = 10, // playful.gif
    DASAI_SURPRISED      = 11, // surprised.gif
    DASAI_EXCITED_BOUNCE = 12, // dancing.gif
    DASAI_SLEEPY_DROOP   = 13, // drowsy.gif
    DASAI_SLEEPING_ZZZ   = 14, // sleepy.gif
    DASAI_ANGRY_GLARE    = 15, // angry.gif
    DASAI_TURBO_RACE     = 16, // fast.gif
    DASAI_DRIFT_G_FORCE  = 17, // smoke.gif
    DASAI_DIZZY_SPIRAL   = 18, // dizzy.gif
    DASAI_SHOCKED_LIGHT  = 19, // devil.gif
    DASAI_MATRIX_VISOR   = 20, // handsome.gif
    DASAI_SAD_TEAR       = 21, // crying.gif
    DASAI_SMUG_CAT       = 22  // giggle.gif
};

// Master Playback & Choreography Modes
enum DasaiCycleMode {
    CYCLE_AUTO_ALL   = 0, // Master Grand Loop through authentic GIFs (72s)
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
    DasaiGifPlayer gifPlayer;

    // 2-Second Hold Secret Reaction Card
    bool         isShyLoveActive   = false;
    uint32_t     shyLoveStartTime  = 0;
    DasaiEmotion preShyEmotion     = DASAI_IDLE_LOOK;
    String       customShyText     = "I LOVE YOU :heart: :sparkles:";

private:
    uint32_t masterCycleStart = 0;
    uint32_t stateStartTime   = 0;
    DasaiEmotion lastLoadedEmotion = (DasaiEmotion)-1;
    String   lastLoadedPath = "";

public:
    DasaiMochi() {
        masterCycleStart = millis();
        stateStartTime = millis();
    }

    void setStyle(int style) {
        currentStyle = (style >= 0 && style < 5) ? style : 0;
        gifPlayer.setStyle(currentStyle);
    }

    int cycleStyle() {
        currentStyle = gifPlayer.cycleStyle();
        return currentStyle;
    }

    void setShyText(const String& txt) {
        if (txt.length() > 0) customShyText = txt;
    }

    void triggerShyLove(uint32_t durMs = 5000) {
        if (!isShyLoveActive) preShyEmotion = currentEmotion;
        isShyLoveActive = true;
        shyLoveStartTime = millis();
        playAuthenticGif("/mochi/love.gif", false);
    }

    void triggerTap() {
        if (currentEmotion == DASAI_HAPPY_SQUINT) {
            playAuthenticGif("/mochi/dancing.gif", false);
            currentEmotion = DASAI_EXCITED_BOUNCE;
        } else {
            playAuthenticGif("/mochi/happy.gif", false);
            currentEmotion = DASAI_HAPPY_SQUINT;
        }
        cycleMode = CYCLE_MANUAL_LOCK;
        stateStartTime = millis();
    }

    void triggerTripleTapTurbo() {
        playAuthenticGif("/mochi/fast.gif", false);
        currentEmotion = DASAI_TURBO_RACE;
        cycleMode = CYCLE_MANUAL_LOCK;
        stateStartTime = millis();
    }

    const char* getGifPathForEmotion(DasaiEmotion emo) {
        switch (emo) {
            case DASAI_IDLE_LOOK:      return "/mochi/blank.gif";
            case DASAI_LOOK_LEFT:      return "/mochi/left.gif";
            case DASAI_LOOK_RIGHT:     return "/mochi/right.gif";
            case DASAI_LOOK_UP:        return "/mochi/serene.gif";
            case DASAI_LOOK_DOWN:      return "/mochi/down.gif";
            case DASAI_HAPPY_SQUINT:   return "/mochi/happy.gif";
            case DASAI_WINK_L:         return "/mochi/wink.gif";
            case DASAI_WINK_R:         return "/mochi/wink.gif";
            case DASAI_HEART_LOVE:     return "/mochi/love.gif";
            case DASAI_SPARKLE_JOY:    return "/mochi/sparkle.gif";
            case DASAI_CURIOUS_TILT:   return "/mochi/playful.gif";
            case DASAI_SURPRISED:      return "/mochi/surprised.gif";
            case DASAI_EXCITED_BOUNCE: return "/mochi/dancing.gif";
            case DASAI_SLEEPY_DROOP:   return "/mochi/drowsy.gif";
            case DASAI_SLEEPING_ZZZ:   return "/mochi/sleepy.gif";
            case DASAI_ANGRY_GLARE:    return "/mochi/angry.gif";
            case DASAI_TURBO_RACE:     return "/mochi/fast.gif";
            case DASAI_DRIFT_G_FORCE:  return "/mochi/smoke.gif";
            case DASAI_DIZZY_SPIRAL:   return "/mochi/dizzy.gif";
            case DASAI_SHOCKED_LIGHT:  return "/mochi/devil.gif";
            case DASAI_MATRIX_VISOR:   return "/mochi/handsome.gif";
            case DASAI_SAD_TEAR:       return "/mochi/crying.gif";
            case DASAI_SMUG_CAT:       return "/mochi/giggle.gif";
            default:                   return "/mochi/blank.gif";
        }
    }

    void setEmotion(DasaiEmotion emo) {
        currentEmotion = emo;
        cycleMode = CYCLE_MANUAL_LOCK;
        stateStartTime = millis();
        loadCurrentEmotionGif(true);
    }

    void playEmotionName(const String& name) {
        String path = "/mochi/" + name + ".gif";
        if (!LittleFS.exists(path)) path = "/" + name + ".gif";
        if (LittleFS.exists(path)) {
            playAuthenticGif(path, (name == "blank"));
            cycleMode = CYCLE_MANUAL_LOCK;
            stateStartTime = millis();
        }
    }

    void setCycleMode(DasaiCycleMode mode) {
        cycleMode = mode;
        masterCycleStart = millis();
        stateStartTime = millis();
        lastLoadedEmotion = (DasaiEmotion)-1;
    }

    void playAuthenticGif(const String& path, bool loop = true) {
        if (LittleFS.exists(path)) {
            gifPlayer.playGif(path, loop, 1);
            lastLoadedPath = path;
        } else if (LittleFS.exists("/mochi/blank.gif")) {
            gifPlayer.playGif("/mochi/blank.gif", true, 1);
            lastLoadedPath = "/mochi/blank.gif";
        }
    }

    void loadCurrentEmotionGif(bool continuous = false) {
        const char* path = getGifPathForEmotion(currentEmotion);
        playAuthenticGif(path, continuous || (currentEmotion == DASAI_IDLE_LOOK));
        lastLoadedEmotion = currentEmotion;
    }

    void update() {
        uint32_t now = millis();

        // Auto revert from shy love after 5 seconds
        if (isShyLoveActive && (now - shyLoveStartTime >= 5000)) {
            isShyLoveActive = false;
            currentEmotion = preShyEmotion;
            loadCurrentEmotionGif(true);
        }

        // =====================================================================
        // CHOREOGRAPHED CYCLE SEQUENCER
        // =====================================================================
        if (!isShyLoveActive && cycleMode != CYCLE_MANUAL_LOCK) {
            executeMasterCycle(now);
            if (currentEmotion != lastLoadedEmotion) {
                loadCurrentEmotionGif(false);
            }
        }

        // =====================================================================
        // RENDER AUTHENTIC GIF SCANLINES DIRECTLY TO FRAMEBUFFER
        // =====================================================================
        gifPlayer.update();

        // Render 2-Second Hold Secret Message Overlay if active
        if (isShyLoveActive) {
            renderShyLoveCard();
        }
    }

private:
    void executeMasterCycle(uint32_t now) {
        if (cycleMode == CYCLE_AUTO_ALL) {
            // Full 72-second master story loop showcasing authentic expressions
            uint32_t cycleTime = (now - masterCycleStart) % 72000;

            if      (cycleTime < 6000)  currentEmotion = DASAI_IDLE_LOOK;      // blank.gif
            else if (cycleTime < 10000) currentEmotion = DASAI_HAPPY_SQUINT;   // happy.gif
            else if (cycleTime < 14000) currentEmotion = DASAI_WINK_L;         // wink.gif
            else if (cycleTime < 18000) currentEmotion = DASAI_SPARKLE_JOY;    // sparkle.gif
            else if (cycleTime < 22000) currentEmotion = DASAI_HEART_LOVE;     // love.gif
            else if (cycleTime < 26000) currentEmotion = DASAI_CURIOUS_TILT;   // playful.gif
            else if (cycleTime < 30000) currentEmotion = DASAI_SURPRISED;      // surprised.gif
            else if (cycleTime < 34000) currentEmotion = DASAI_EXCITED_BOUNCE; // dancing.gif
            else if (cycleTime < 38000) currentEmotion = DASAI_ANGRY_GLARE;    // angry.gif
            else if (cycleTime < 43000) currentEmotion = DASAI_TURBO_RACE;     // fast.gif
            else if (cycleTime < 48000) currentEmotion = DASAI_DRIFT_G_FORCE;  // smoke.gif
            else if (cycleTime < 52500) currentEmotion = DASAI_DIZZY_SPIRAL;   // dizzy.gif
            else if (cycleTime < 56500) currentEmotion = DASAI_SMUG_CAT;       // giggle.gif
            else if (cycleTime < 61000) currentEmotion = DASAI_SLEEPY_DROOP;   // drowsy.gif
            else if (cycleTime < 67000) currentEmotion = DASAI_SLEEPING_ZZZ;   // sleepy.gif
            else                        currentEmotion = DASAI_SAD_TEAR;       // crying.gif

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
