#pragma once
#include <LovyanGFX.hpp>
#include "display_setup.h"
#include "emotion_assets.h"
#include "config.h"

enum MemeEmotion {
    EMOTION_LUFFY = 0,       // Default / Star-Eyes / Excited
    EMOTION_SHY = 1,         // Shy Love (Rub / Hold)
    EMOTION_GIGGLE_CAT = 2,  // Giggling Kitten with Pink Bows
    EMOTION_SAD_BANANA = 3,  // Sad / Crying Banana Cat
    EMOTION_UMARU_CRY = 4,   // Whining Dramatic Umaru
    EMOTION_ANGRY_CAT = 5,   // Angry / Grumpy Kitten
    EMOTION_BUNNY = 6        // Pouting Bunny "Hum!"
};

class MemePet {
public:
    MemeEmotion defaultEmotion = EMOTION_LUFFY;
    MemeEmotion currentEmotion = EMOTION_LUFFY;
    uint32_t emotionStartTime = 0;
    int animTick = 0;
    uint32_t lastAnimTime = 0;

    void setDefaultEmotion(MemeEmotion emo) {
        defaultEmotion = emo;
        currentEmotion = emo;
    }

    void setEmotion(MemeEmotion emo) {
        currentEmotion = emo;
        emotionStartTime = millis();
    }

    void triggerTap() {
        emotionStartTime = millis();
    }

    void triggerHold() {
        setEmotion(EMOTION_SHY);
    }

    void triggerDoubleTap() {
        int next = ((int)currentEmotion + 1) % 7;
        setEmotion((MemeEmotion)next);
        defaultEmotion = (MemeEmotion)next;
    }

    void update() {
        uint32_t now = millis();

        if (now - lastAnimTime > 50) {
            lastAnimTime = now;
            animTick = (animTick + 1) % 120;
        }

        // 1. Draw selected Meme Emotion JPEG
        switch (currentEmotion) {
            case EMOTION_LUFFY:
                canvas.drawJpg(EMO_LUFFY_JPG, EMO_LUFFY_LEN, 0, 0);
                drawLuffyJoyEffects();
                break;
            case EMOTION_SHY:
                canvas.drawJpg(EMO_SHY_JPG, EMO_SHY_LEN, 0, 0);
                drawFloatingHearts();
                break;
            case EMOTION_GIGGLE_CAT:
                canvas.drawJpg(EMO_GIGGLE_CAT_JPG, EMO_GIGGLE_CAT_LEN, 0, 0);
                drawGiggleStars();
                break;
            case EMOTION_SAD_BANANA:
                canvas.drawJpg(EMO_SAD_BANANA_JPG, EMO_SAD_BANANA_LEN, 0, 0);
                drawTears();
                break;
            case EMOTION_UMARU_CRY:
                canvas.drawJpg(EMO_UMARU_JPG, EMO_UMARU_LEN, 0, 0);
                drawWhineEffects();
                break;
            case EMOTION_ANGRY_CAT:
                canvas.drawJpg(EMO_ANGRY_CAT_JPG, EMO_ANGRY_CAT_LEN, 0, 0);
                drawAngerSparks();
                break;
            case EMOTION_BUNNY:
                canvas.drawJpg(EMO_BUNNY_JPG, EMO_BUNNY_LEN, 0, 0);
                drawAngerSparks();
                break;
        }

        // 2. Sleek Outer Border
        drawBorder();
    }

private:
    void drawBorder() {
        uint16_t borderCol = 0xFD00; // Warm Amber Gold for Luffy!
        if (currentEmotion == EMOTION_SHY || currentEmotion == EMOTION_GIGGLE_CAT) borderCol = 0xF81F; // Pink
        else if (currentEmotion == EMOTION_ANGRY_CAT || currentEmotion == EMOTION_BUNNY) borderCol = 0xF800; // Red
        else if (currentEmotion == EMOTION_SAD_BANANA || currentEmotion == EMOTION_UMARU_CRY) borderCol = 0x07FF; // Blue

        canvas.drawRoundRect(0, 0, 240, 240, 6, borderCol);
        canvas.drawRoundRect(1, 1, 238, 238, 5, borderCol);
    }

    void drawLuffyJoyEffects() {
        // 1. Floating & Twinkling Gold Sparkle Stars
        // Star 1 (Top Left)
        int s1x = 32 + (int)(sin(animTick * 0.12f) * 6);
        int s1y = 36 + (int)(cos(animTick * 0.15f) * 8);
        int s1Flare = 4 + (int)(sin(animTick * 0.25f) * 3);
        canvas.fillCircle(s1x, s1y, 3, 0xFFE0); // Gold
        canvas.drawFastHLine(s1x - s1Flare, s1y, s1Flare * 2 + 1, 0xFFFF); // White flare
        canvas.drawFastVLine(s1x, s1y - s1Flare, s1Flare * 2 + 1, 0xFFFF);
        canvas.fillCircle(s1x, s1y, 1, 0xFFFF);

        // Star 2 (Top Right)
        int s2x = 208 + (int)(cos(animTick * 0.14f) * 6);
        int s2y = 38 + (int)(sin(animTick * 0.18f) * 7);
        int s2Flare = 4 + (int)(cos(animTick * 0.22f) * 3);
        canvas.fillCircle(s2x, s2y, 3, 0xFFE0);
        canvas.drawFastHLine(s2x - s2Flare, s2y, s2Flare * 2 + 1, 0xFFFF);
        canvas.drawFastVLine(s2x, s2y - s2Flare, s2Flare * 2 + 1, 0xFFFF);
        canvas.fillCircle(s2x, s2y, 1, 0xFFFF);

        // Star 3 (Mid Left Micro Star)
        int s3x = 24 + (int)(sin((animTick + 40) * 0.15f) * 4);
        int s3y = 88 + (int)(cos((animTick + 40) * 0.18f) * 6);
        canvas.fillCircle(s3x, s3y, 2, 0xFFE0);
        canvas.drawFastHLine(s3x - 3, s3y, 7, 0xFFE0);
        canvas.drawFastVLine(s3x, s3y - 3, 7, 0xFFE0);

        // 2. Playful Floating Hearts
        // Heart 1 (Upper Right Side)
        int h1x = 205 + (int)(sin(animTick * 0.10f) * 5);
        int h1y = 90 + (int)(cos(animTick * 0.13f) * 9);
        canvas.fillCircle(h1x - 3, h1y - 2, 3, 0xF800);
        canvas.fillCircle(h1x + 3, h1y - 2, 3, 0xF800);
        canvas.fillTriangle(h1x - 6, h1y - 1, h1x + 6, h1y - 1, h1x, h1y + 6, 0xF800);
        canvas.fillCircle(h1x - 2, h1y - 2, 1, 0xFBE4); // Highlight

        // Heart 2 (Mid-Low Left Side)
        int h2x = 34 + (int)(cos((animTick + 30) * 0.12f) * 5);
        int h2y = 145 + (int)(sin((animTick + 30) * 0.16f) * 7);
        canvas.fillCircle(h2x - 2, h2y - 2, 2, 0xF81F);
        canvas.fillCircle(h2x + 2, h2y - 2, 2, 0xF81F);
        canvas.fillTriangle(h2x - 4, h2y - 1, h2x + 4, h2y - 1, h2x, h2y + 4, 0xF81F);

        // 3. Comic-Book Energetic Exclamation Marks '!!'
        int ex1 = 175 + (int)(sin(animTick * 0.2f) * 3);
        int ey1 = 20 + (int)(cos(animTick * 0.2f) * 3);
        canvas.fillRoundRect(ex1, ey1, 3, 9, 1, 0xFD20); // Amber bar
        canvas.fillCircle(ex1 + 1, ey1 + 13, 2, 0xFD20); // Amber dot
        canvas.fillRoundRect(ex1 + 7, ey1 - 2, 3, 10, 1, 0xFFE0); // Gold bar
        canvas.fillCircle(ex1 + 8, ey1 + 12, 2, 0xFFE0); // Gold dot

        // Action / Excitement Spark near Straw Hat
        int spx = 120 + (int)(sin(animTick * 0.25f) * 14);
        int spy = 12 + (int)(cos(animTick * 0.25f) * 4);
        canvas.drawFastHLine(spx - 4, spy, 9, 0x07FF);
        canvas.drawFastVLine(spx, spy - 4, 9, 0x07FF);
    }

    void drawFloatingHearts() {
        for (int i = 0; i < 3; i++) {
            int hx = 30 + i * 85 + (int)(sin((animTick + i * 20) * 0.1f) * 10);
            int hy = 40 + (int)(cos((animTick + i * 20) * 0.15f) * 12);
            canvas.fillCircle(hx - 3, hy - 2, 4, 0xF81F);
            canvas.fillCircle(hx + 3, hy - 2, 4, 0xF81F);
            canvas.fillTriangle(hx - 7, hy - 1, hx + 7, hy - 1, hx, hy + 7, 0xF81F);
        }
    }

    void drawGiggleStars() {
        for (int i = 0; i < 2; i++) {
            int sx = (i == 0) ? 35 : 205;
            int sy = 35 + (int)(sin((animTick + i * 30) * 0.2f) * 8);
            canvas.fillCircle(sx, sy, 3, 0xFFE0);
            canvas.drawFastHLine(sx - 6, sy, 13, 0xFFE0);
            canvas.drawFastVLine(sx, sy - 6, 13, 0xFFE0);
        }
    }

    void drawAngerSparks() {
        int sx = 200 + (int)(sin(animTick * 0.3f) * 4);
        int sy = 35 + (int)(cos(animTick * 0.3f) * 4);
        canvas.drawFastHLine(sx - 8, sy - 3, 16, 0xF800);
        canvas.drawFastHLine(sx - 8, sy + 3, 16, 0xF800);
        canvas.drawFastVLine(sx - 3, sy - 8, 16, 0xF800);
        canvas.drawFastVLine(sx + 3, sy - 8, 16, 0xF800);
    }

    void drawTears() {
        int ty = 140 + (animTick % 30) * 2;
        canvas.fillCircle(85, ty, 3, 0x07FF);
        canvas.fillCircle(155, ty, 3, 0x07FF);
    }

    void drawWhineEffects() {
        int ty = 110 + (animTick % 25) * 3;
        canvas.fillCircle(70, ty, 4, 0x07FF);
        canvas.fillCircle(170, ty, 4, 0x07FF);
    }
};
