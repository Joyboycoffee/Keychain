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

    void update() {
        uint32_t now = millis();

        // Auto-return to default emotion after 4 seconds of idle reaction
        if (currentEmotion != defaultEmotion && (now - emotionStartTime > 4000)) {
            currentEmotion = defaultEmotion;
        }

        if (now - lastAnimTime > 50) {
            lastAnimTime = now;
            animTick = (animTick + 1) % 120;
        }

        // 1. Draw selected Meme Emotion JPEG
        switch (currentEmotion) {
            case EMOTION_LUFFY:
                canvas.drawJpg(EMO_LUFFY_JPG, EMO_LUFFY_LEN, 0, 0);
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
        uint16_t borderCol = 0x07FF; // Cyan
        if (currentEmotion == EMOTION_SHY || currentEmotion == EMOTION_GIGGLE_CAT) borderCol = 0xF81F; // Pink
        else if (currentEmotion == EMOTION_ANGRY_CAT || currentEmotion == EMOTION_BUNNY) borderCol = 0xF800; // Red
        else if (currentEmotion == EMOTION_SAD_BANANA || currentEmotion == EMOTION_UMARU_CRY) borderCol = 0x001F; // Blue

        canvas.drawRoundRect(0, 0, 240, 240, 6, borderCol);
        canvas.drawRoundRect(1, 1, 238, 238, 5, borderCol);
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
