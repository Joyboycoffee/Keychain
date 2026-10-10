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

struct MochiAnimItem {
    const char* path;
    const char* name;
};

static const MochiAnimItem MOCHI_ANIM_CATALOG[] = {
    { "/mochi/blank.gif",     "BLANK IDLE ⚪" },
    { "/mochi/happy.gif",     "HAPPY SQUINT 😊" },
    { "/mochi/wink.gif",      "PLAYFUL WINK 😉" },
    { "/mochi/love.gif",      "HEART LOVE ❤️" },
    { "/mochi/sparkle.gif",   "SPARKLE JOY ✨" },
    { "/mochi/playful.gif",   "PLAYFUL TILT 😋" },
    { "/mochi/surprised.gif", "SURPRISED 😮" },
    { "/mochi/dancing.gif",   "DANCING BOUNCE 💃" },
    { "/mochi/angry.gif",     "ANGRY GLARE 😾" },
    { "/mochi/fast.gif",      "TURBO RACE 🏁" },
    { "/mochi/smoke.gif",     "DRIFT SMOKE 💨" },
    { "/mochi/dizzy.gif",     "DIZZY SPIRAL 😵" },
    { "/mochi/giggle.gif",    "GIGGLE LAUGH 😸" },
    { "/mochi/drowsy.gif",    "DROWSY SLEEPY 🥱" },
    { "/mochi/sleepy.gif",    "SLEEPY ZZZ 💤" },
    { "/mochi/crying.gif",    "CRYING TEARS 😭" },
    { "/mochi/devil.gif",     "DEVIL HORNS 😈" },
    { "/mochi/handsome.gif",  "HANDSOME CHAD 😎" },
    { "/mochi/yawn.gif",      "BIG YAWN 🥱" },
    { "/mochi/smile.gif",     "WARM SMILE 🙂" },
    { "/mochi/hello.gif",     "HELLO WAVE 👋" },
    { "/mochi/laughing.gif",  "LAUGHING OUT LOUD 😆" },
    { "/mochi/adore.gif",     "ADORE PET 😻" },
    { "/mochi/brave.gif",     "BRAVE HERO 🦸" },
    { "/mochi/rush.gif",      "SPEED RUSH 🏎️" },
    { "/mochi/serene.gif",    "SERENE CHILL 🧘" },
    { "/mochi/shy.gif",       "SHY BLUSH 👉👈" },
    { "/mochi/sick.gif",      "SICK WHEEZE 🤢" },
    { "/mochi/sneeze.gif",    "SNEEZE ACHOO 🤧" },
    { "/mochi/sobbing.gif",   "SOBBING CRY 💧" },
    { "/mochi/splash.gif",    "WATER SPLASH 💦" },
    { "/mochi/spraying.gif",  "SPRAY CLEAN 🚿" },
    { "/mochi/squint.gif",    "COOL SQUINT 😏" },
    { "/mochi/teasing.gif",   "TEASING TONGUE 😜" },
    { "/mochi/contempt.gif",  "SIDE EYE 👀" },
    { "/mochi/enraged.gif",   "ENRAGED RAGE 🤬" },
    { "/mochi/fierce.gif",    "FIERCE BATTLE ⚔️" },
    { "/mochi/growing.gif",   "GROWING POWER 🌟" },
    { "/mochi/left.gif",      "LOOK LEFT 👈" },
    { "/mochi/right.gif",     "LOOK RIGHT 👉" },
    { "/mochi/down.gif",      "LOOK DOWN 👇" },
    { "/mochi/relaxed.gif",   "CHILL RELAXED ☕" }
};
static const int MOCHI_ANIM_COUNT = sizeof(MOCHI_ANIM_CATALOG) / sizeof(MOCHI_ANIM_CATALOG[0]);

class DasaiMochi {
public:
    DasaiEmotion   currentEmotion = DASAI_IDLE_LOOK;
    DasaiCycleMode cycleMode      = CYCLE_AUTO_ALL;
    int            currentStyle   = 0; // 0=Warm White, 1=Pastel Cyan, 2=Sakura Pink, 3=Mochi Gold, 4=Neon Violet
    int            currentAnimIdx = 0;
    DasaiGifPlayer gifPlayer;

    // 2-Second Hold Secret Reaction Card
    bool         isShyLoveActive   = false;
    uint32_t     shyLoveStartTime  = 0;
    DasaiEmotion preShyEmotion     = DASAI_IDLE_LOOK;
    String       customShyText     = "I LOVE YOU :heart: :sparkles:";

private:
    uint32_t masterCycleStart       = 0;
    uint32_t stateStartTime         = 0;
    uint32_t currentAnimStartTime   = 0;
    int      showcaseIdx            = 0;
    DasaiEmotion lastLoadedEmotion  = (DasaiEmotion)-1;
    String   lastLoadedPath         = "";

public:
    DasaiMochi() {
        masterCycleStart = millis();
        stateStartTime = millis();
        currentAnimStartTime = millis();
    }

    void setStyle(int style) {
        currentStyle = (style >= 0 && style < 5) ? style : 0;
        gifPlayer.setStyle(currentStyle);
    }

    int cycleStyle() {
        currentStyle = gifPlayer.cycleStyle();
        return currentStyle;
    }

    // 2 Taps Action: Change animations one by one (DO NOT cycle colors!)
    const char* cycleNextAnimation() {
        currentAnimIdx = (currentAnimIdx + 1) % MOCHI_ANIM_COUNT;
        showcaseIdx = currentAnimIdx;
        cycleMode = CYCLE_MANUAL_LOCK;
        const MochiAnimItem& item = MOCHI_ANIM_CATALOG[currentAnimIdx];
        playAuthenticGif(item.path, true);
        stateStartTime = millis();
        currentAnimStartTime = millis();
        Serial.printf("[MOCHI] 2 Taps -> Animation %d/%d: %s (%s)\n", currentAnimIdx + 1, MOCHI_ANIM_COUNT, item.name, item.path);
        return item.name;
    }

    const char* getCurrentAnimationName() const {
        if (currentAnimIdx >= 0 && currentAnimIdx < MOCHI_ANIM_COUNT) {
            return MOCHI_ANIM_CATALOG[currentAnimIdx].name;
        }
        return "ROBOT EYES 🤖";
    }

    const char* getCurrentAnimationPath() const {
        if (currentAnimIdx >= 0 && currentAnimIdx < MOCHI_ANIM_COUNT) {
            return MOCHI_ANIM_CATALOG[currentAnimIdx].path;
        }
        return "/mochi/blank.gif";
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
            for (int i = 0; i < MOCHI_ANIM_COUNT; i++) {
                if (String(MOCHI_ANIM_CATALOG[i].path).endsWith(name + ".gif")) {
                    currentAnimIdx = i;
                    showcaseIdx = i;
                    break;
                }
            }
        }
    }

    void setCycleMode(DasaiCycleMode mode) {
        cycleMode = mode;
        masterCycleStart = millis();
        stateStartTime = millis();
        currentAnimStartTime = millis();
        lastLoadedEmotion = (DasaiEmotion)-1;
        if (cycleMode == CYCLE_AUTO_ALL) {
            showcaseIdx = 0;
            currentAnimIdx = 0;
            playAuthenticGif(MOCHI_ANIM_CATALOG[showcaseIdx].path, false);
        }
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
        // CHOREOGRAPHED CYCLE SEQUENCER (FULL COMPLETION - NEVER CUT IN MIDDLE)
        // =====================================================================
        if (!isShyLoveActive && cycleMode != CYCLE_MANUAL_LOCK) {
            executeMasterCycle(now);
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
            // Grand Showcase: Play EACH animation FULLY to the end! Do NOT cut in the middle!
            bool isIdleFace = (showcaseIdx == 0 || strcmp(MOCHI_ANIM_CATALOG[showcaseIdx].path, "/mochi/blank.gif") == 0);

            bool shouldAdvance = false;
            if (isIdleFace) {
                // Calm idle face pause for 3.5s
                if (now - currentAnimStartTime >= 3500) {
                    shouldAdvance = true;
                }
            } else {
                // Action GIF: MUST finish entire animation to the last frame!
                if (gifPlayer.checkAndClearLoopFinished()) {
                    shouldAdvance = true;
                }
            }

            if (shouldAdvance) {
                showcaseIdx = (showcaseIdx + 1) % MOCHI_ANIM_COUNT;
                currentAnimIdx = showcaseIdx;
                currentAnimStartTime = now;
                const MochiAnimItem& nextItem = MOCHI_ANIM_CATALOG[showcaseIdx];
                bool nextIsIdle = (showcaseIdx == 0 || strcmp(nextItem.path, "/mochi/blank.gif") == 0);
                playAuthenticGif(nextItem.path, nextIsIdle);
            }
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
