#pragma once
#include <LovyanGFX.hpp>
#include "display_setup.h"
#include "emotion_assets.h"
#include "config.h"

enum MemeEmotion {
    EMOTION_LUFFY = 0,       // Default / Star-Eyes / Excited
    EMOTION_SHY = 1,         // Shy Love `????` (Rub / Hold)
    EMOTION_SAD_BANANA = 2,  // Sad / Crying Banana Cat
    EMOTION_ANGRY_CAT = 3,   // Angry / Grumpy Kitten
    EMOTION_BUNNY = 4        // Pouting Bunny "Hum!"
};

class MemePet {
public:
    MemeEmotion currentEmotion = EMOTION_LUFFY;
    uint32_t emotionStartTime = 0;
    int animTick = 0;
    uint32_t lastAnimTime = 0;

    void setEmotion(MemeEmotion emo) {
        currentEmotion = emo;
        emotionStartTime = millis();
    }

    void update() {
        uint32_t now = millis();

        // If in temporary emotion (shy, angry, sad), return to Luffy after 6 seconds of inactivity
        if (currentEmotion != EMOTION_LUFFY && (now - emotionStartTime > 6000)) {
            currentEmotion = EMOTION_LUFFY;
        }

        if (now - lastAnimTime > 50) {
            lastAnimTime = now;
            animTick = (animTick + 1) % 120;
        }

        // 1. Draw the selected Meme Emotion JPEG
        switch (currentEmotion) {
            case EMOTION_LUFFY:
                canvas.drawJpg(EMO_LUFFY_JPG, EMO_LUFFY_LEN, 0, 0, 240, 240);
                break;
            case EMOTION_SHY:
                canvas.drawJpg(EMO_SHY_JPG, EMO_SHY_LEN, 0, 0, 240, 240);
                drawFloatingHearts();
                break;
            case EMOTION_SAD_BANANA:
                canvas.drawJpg(EMO_SAD_BANANA_JPG, EMO_SAD_BANANA_LEN, 0, 0, 240, 240);
                drawTears();
                break;
            case EMOTION_ANGRY_CAT:
                canvas.drawJpg(EMO_ANGRY_CAT_JPG, EMO_ANGRY_CAT_LEN, 0, 0, 240, 240);
                drawAngerSparks();
                break;
            case EMOTION_BUNNY:
                canvas.drawJpg(EMO_BUNNY_JPG, EMO_BUNNY_LEN, 0, 0, 240, 240);
                drawAngerSparks();
                break;
        }

        // 2. Sleek Aesthetic Outer Border
        drawBorder();
    }

private:
    void drawBorder() {
        uint16_t borderCol = 0x07FF; // Cyan
        if (currentEmotion == EMOTION_SHY) borderCol = 0xF81F; // Pink
        else if (currentEmotion == EMOTION_ANGRY_CAT || currentEmotion == EMOTION_BUNNY) borderCol = 0xF800; // Red
        else if (currentEmotion == EMOTION_SAD_BANANA) borderCol = 0x001F; // Blue

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

    void drawAngerSparks() {
        int sx = 200 + (int)(sin(animTick * 0.3f) * 4);
        int sy = 35 + (int)(cos(animTick * 0.3f) * 4);
        canvas.setTextColor(0xF800, TFT_WHITE);
        canvas.setTextSize(2);
        canvas.drawString("??", sx, sy);
    }

    void drawTears() {
        int ty = 140 + (animTick % 30) * 2;
        canvas.fillCircle(85, ty, 3, 0x07FF);
        canvas.fillCircle(155, ty, 3, 0x07FF);
    }
};
