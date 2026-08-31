#pragma once
#include <LovyanGFX.hpp>
#include "display_setup.h"
#include "config.h"

class CyberPet {
public:
    PetAvatar currentAvatar = PET_CYBER_CAT;
    PetMood mood = MOOD_IDLE;
    uint32_t moodStartTime = 0;
    int animFrame = 0;
    uint32_t lastAnimTick = 0;

    void setAvatar(PetAvatar avatar) {
        currentAvatar = avatar;
    }

    void setMood(PetMood newMood) {
        mood = newMood;
        moodStartTime = millis();
    }

    void update() {
        uint32_t now = millis();

        // Mood expiration (return to IDLE after 4 seconds of happy/angry)
        if (mood != MOOD_IDLE && (now - moodStartTime > 4000)) {
            mood = MOOD_IDLE;
        }

        // Frame ticker (10 FPS sprite step)
        if (now - lastAnimTick > 100) {
            lastAnimTick = now;
            animFrame = (animFrame + 1) % 60;
        }

        canvas.fillScreen(0x0821); // Dark Cyber Navy/Black

        // Render ambient sci-fi grid
        drawCyberGrid();

        // Render selected avatar
        switch (currentAvatar) {
            case PET_CYBER_CAT:
                drawCyberCat(120, 130 + sin(animFrame * 0.15f) * 4);
                break;
            case PET_MECH_BOT:
                drawMechBot(120, 125 + cos(animFrame * 0.2f) * 6);
                break;
            case PET_PIXEL_DRAGON:
                drawPixelDragon(120, 130 + sin(animFrame * 0.12f) * 5);
                break;
        }

        // Render Status Bar / Mood Badge
        drawStatusBadge();
    }

private:
    void drawCyberGrid() {
        for (int y = 200; y < 240; y += 10) {
            canvas.drawFastHLine(0, y, 240, 0x18C3);
        }
        for (int x = 20; x < 240; x += 30) {
            canvas.drawLine(x, 200, x + (x - 120) * 0.5f, 240, 0x18C3);
        }
    }

    void drawCyberCat(int cx, int cy) {
        uint16_t catColor = 0xFFFF;       // White Cyber Cat
        uint16_t earPink  = 0xF81F;       // Neon Pink Inner Ears
        uint16_t visorCol = (mood == MOOD_ANGRY) ? 0xF800 : ((mood == MOOD_HAPPY) ? 0xF81F : 0x07FF);

        // Body & Head
        canvas.fillRoundRect(cx - 38, cy - 25, 76, 60, 24, catColor);
        canvas.fillCircle(cx, cy - 18, 36, catColor);

        // Cyber Ears (Triangles)
        int earWiggle = (mood == MOOD_ANGRY) ? random(-3, 4) : 0;
        canvas.fillTriangle(cx - 32, cy - 35, cx - 12, cy - 50 + earWiggle, cx - 6, cy - 30, catColor);
        canvas.fillTriangle(cx + 32, cy - 35, cx + 12, cy - 50 - earWiggle, cx + 6, cy - 30, catColor);
        canvas.fillTriangle(cx - 28, cy - 34, cx - 14, cy - 45 + earWiggle, cx - 10, cy - 32, earPink);
        canvas.fillTriangle(cx + 28, cy - 34, cx + 14, cy - 45 - earWiggle, cx + 10, cy - 32, earPink);

        // Visor / Eyes
        if (mood == MOOD_HAPPY) {
            // Heart Eyes
            canvas.fillCircle(cx - 16, cy - 18, 7, visorCol);
            canvas.fillCircle(cx - 6, cy - 18, 7, visorCol);
            canvas.fillTriangle(cx - 23, cy - 16, cx + 1, cy - 16, cx - 11, cy - 4, visorCol);

            canvas.fillCircle(cx + 6, cy - 18, 7, visorCol);
            canvas.fillCircle(cx + 16, cy - 18, 7, visorCol);
            canvas.fillTriangle(cx - 1, cy - 16, cx + 23, cy - 16, cx + 11, cy - 4, visorCol);

            // Blush Cheeks
            canvas.fillCircle(cx - 26, cy - 6, 6, 0xFA48);
            canvas.fillCircle(cx + 26, cy - 6, 6, 0xFA48);
        } else if (mood == MOOD_ANGRY) {
            // Angry Eyes
            canvas.fillTriangle(cx - 26, cy - 25, cx - 6, cy - 15, cx - 26, cy - 12, visorCol);
            canvas.fillTriangle(cx + 26, cy - 25, cx + 6, cy - 15, cx + 26, cy - 12, visorCol);

            // Angry Steam Sparks
            canvas.drawString("💢", cx + 25, cy - 50);
        } else {
            // Normal Cute Cyber Eyes (blinks periodically)
            bool blink = ((animFrame % 40) > 36);
            if (blink) {
                canvas.drawFastHLine(cx - 24, cy - 18, 14, visorCol);
                canvas.drawFastHLine(cx + 10, cy - 18, 14, visorCol);
            } else {
                canvas.fillRoundRect(cx - 24, cy - 24, 14, 16, 6, visorCol);
                canvas.fillRoundRect(cx + 10, cy - 24, 14, 16, 6, visorCol);
                canvas.fillCircle(cx - 20, cy - 21, 3, TFT_WHITE);
                canvas.fillCircle(cx + 14, cy - 21, 3, TFT_WHITE);
            }
        }

        // Cute Mouth & Nose
        canvas.fillTriangle(cx - 3, cy - 8, cx + 3, cy - 8, cx, cy - 5, earPink);
        canvas.drawCircle(cx - 6, cy - 2, 5, 0x39E7);
        canvas.drawCircle(cx + 6, cy - 2, 5, 0x39E7);
        canvas.fillRect(cx - 12, cy - 7, 24, 5, catColor); // mask top circle

        // Animated Tail
        int tailOffset = sin(animFrame * 0.3f) * 14;
        canvas.drawLine(cx + 30, cy + 20, cx + 50 + tailOffset, cy + 5, catColor);
        canvas.drawLine(cx + 31, cy + 20, cx + 51 + tailOffset, cy + 5, catColor);
        canvas.fillCircle(cx + 50 + tailOffset, cy + 5, 4, catColor);
    }

    void drawMechBot(int cx, int cy) {
        uint16_t botMetal = 0x9CD3;       // Silver Blue
        uint16_t glow     = (mood == MOOD_ANGRY) ? 0xF800 : ((mood == MOOD_HAPPY) ? 0x07E0 : 0x07FF);

        // Antenna
        canvas.drawFastVLine(cx, cy - 50, 16, botMetal);
        canvas.fillCircle(cx, cy - 52, 5, glow);

        // Bot Head Frame
        canvas.fillRoundRect(cx - 40, cy - 34, 80, 58, 14, botMetal);
        canvas.fillRoundRect(cx - 32, cy - 26, 64, 42, 8, TFT_BLACK); // Screen bevel

        // Screen HUD
        if (mood == MOOD_HAPPY) {
            canvas.drawString("^_^", cx - 18, cy - 14);
        } else if (mood == MOOD_ANGRY) {
            canvas.drawString(">_<", cx - 18, cy - 14);
        } else {
            canvas.fillCircle(cx - 16, cy - 6, 8, glow);
            canvas.fillCircle(cx + 16, cy - 6, 8, glow);
            canvas.fillCircle(cx - 18, cy - 8, 3, TFT_WHITE);
            canvas.fillCircle(cx + 14, cy - 8, 3, TFT_WHITE);
        }

        // Thruster Hover Jet
        int flameH = 8 + (animFrame % 6) * 3;
        canvas.fillTriangle(cx - 12, cy + 25, cx + 12, cy + 25, cx, cy + 25 + flameH, 0xFDA0);
        canvas.fillTriangle(cx - 6, cy + 25, cx + 6, cy + 25, cx, cy + 25 + (flameH / 2), 0xFFE0);
    }

    void drawPixelDragon(int cx, int cy) {
        uint16_t dragonCol = 0x07E0; // Neon Green
        uint16_t wingCol   = 0x05E0;

        // Wings wiggling
        int wingFlap = sin(animFrame * 0.4f) * 12;
        canvas.fillTriangle(cx - 20, cy - 10, cx - 45, cy - 30 + wingFlap, cx - 25, cy + 10, wingCol);
        canvas.fillTriangle(cx + 20, cy - 10, cx + 45, cy - 30 + wingFlap, cx + 25, cy + 10, wingCol);

        // Body
        canvas.fillCircle(cx, cy, 32, dragonCol);
        canvas.fillRoundRect(cx - 20, cy - 35, 40, 35, 12, dragonCol);

        // Horns
        canvas.fillTriangle(cx - 16, cy - 30, cx - 24, cy - 48, cx - 8, cy - 32, 0xFFE0);
        canvas.fillTriangle(cx + 16, cy - 30, cx + 24, cy - 48, cx + 8, cy - 32, 0xFFE0);

        // Eyes
        canvas.fillCircle(cx - 10, cy - 18, 5, TFT_WHITE);
        canvas.fillCircle(cx + 10, cy - 18, 5, TFT_WHITE);
        canvas.fillCircle(cx - 9, cy - 18, 2, TFT_BLACK);
        canvas.fillCircle(cx + 11, cy - 18, 2, TFT_BLACK);

        if (mood == MOOD_ANGRY) {
            // Tiny fire breath
            canvas.fillCircle(cx, cy + 15, 8, 0xF800);
            canvas.fillCircle(cx, cy + 22, 5, 0xFDA0);
        }
    }

    void drawStatusBadge() {
        canvas.setTextSize(1);
        canvas.setTextColor(TFT_WHITE, 0x0821);
        const char* moodNames[] = {"IDLE", "HAPPY ^_^", "ANGRY >_<", "SLEEPING zZ", "LOVING <3"};
        canvas.drawCenterString(moodNames[mood], 120, 15);
    }
};
