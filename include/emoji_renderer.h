#pragma once
#include <LovyanGFX.hpp>

enum EmojiType {
    EMOJI_NONE = 0,
    EMOJI_COFFEE,
    EMOJI_HEART,
    EMOJI_FIRE,
    EMOJI_STAR,
    EMOJI_SPARKLES,
    EMOJI_CAT,
    EMOJI_CRY,
    EMOJI_BANANA,
    EMOJI_SKULL,
    EMOJI_ROCKET
};

class EmojiRenderer {
public:
    static void drawEmoji(LGFX_Sprite* canvas, EmojiType type, int x, int y) {
        if (!canvas) return;

        switch (type) {
            case EMOJI_COFFEE: // ☕ Coffee Cup
                canvas->fillRoundRect(x + 2, y + 6, 16, 14, 3, 0xD440); // Brown cup
                canvas->fillRect(x + 3, y + 7, 14, 3, 0x5180); // Dark Coffee liquid
                canvas->drawRoundRect(x + 16, y + 8, 6, 10, 2, 0xD440); // Handle
                canvas->drawFastVLine(x + 5, y + 1, 3, 0xFFFF);
                canvas->drawFastVLine(x + 9, y + 0, 4, 0xFFFF);
                canvas->drawFastVLine(x + 13, y + 2, 3, 0xFFFF);
                break;

            case EMOJI_HEART: // ❤️ Glowing Red Heart
                canvas->fillCircle(x + 6, y + 7, 5, 0xF800);
                canvas->fillCircle(x + 16, y + 7, 5, 0xF800);
                canvas->fillTriangle(x + 1, y + 8, x + 21, y + 8, x + 11, y + 20, 0xF800);
                canvas->fillCircle(x + 5, y + 6, 2, 0xFBE4);
                break;

            case EMOJI_FIRE: // 🔥 Flame
                canvas->fillTriangle(x + 11, y + 1, x + 3, y + 21, x + 19, y + 21, 0xF800);
                canvas->fillTriangle(x + 11, y + 6, x + 5, y + 21, x + 17, y + 21, 0xFD20);
                canvas->fillTriangle(x + 11, y + 11, x + 7, y + 21, x + 15, y + 21, 0xFFE0);
                break;

            case EMOJI_STAR: // ⭐ Gold Star
                canvas->fillTriangle(x + 11, y + 1, x + 2, y + 19, x + 20, y + 19, 0xFFE0);
                canvas->fillTriangle(x + 11, y + 19, x + 2, y + 7, x + 20, y + 7, 0xFFE0);
                canvas->fillCircle(x + 11, y + 11, 4, 0xFFFF);
                break;

            case EMOJI_SPARKLES: // ✨ Sparkles
                canvas->fillCircle(x + 8, y + 8, 3, 0x07FF);
                canvas->drawFastHLine(x + 2, y + 8, 13, 0x07FF);
                canvas->drawFastVLine(x + 8, y + 2, 13, 0x07FF);
                canvas->fillCircle(x + 17, y + 16, 2, 0xFFE0);
                canvas->drawFastHLine(x + 13, y + 16, 9, 0xFFE0);
                canvas->drawFastVLine(x + 17, y + 12, 9, 0xFFE0);
                break;

            case EMOJI_CAT: // 🐱 Cat Face
                canvas->fillCircle(x + 11, y + 12, 8, 0xFD20);
                canvas->fillTriangle(x + 3, y + 7, x + 7, y + 1, x + 9, y + 7, 0xFD20);
                canvas->fillTriangle(x + 13, y + 7, x + 15, y + 1, x + 19, y + 7, 0xFD20);
                canvas->fillCircle(x + 8, y + 11, 1, 0x0000);
                canvas->fillCircle(x + 14, y + 11, 1, 0x0000);
                canvas->fillTriangle(x + 10, y + 14, x + 12, y + 14, x + 11, y + 16, 0xF81F);
                break;

            case EMOJI_CRY: // 😭 Crying Face
                canvas->fillCircle(x + 11, y + 11, 10, 0xFFE0);
                canvas->drawFastHLine(x + 5, y + 9, 4, 0x0000);
                canvas->drawFastHLine(x + 13, y + 9, 4, 0x0000);
                canvas->fillRoundRect(x + 8, y + 14, 6, 4, 1, 0x0000);
                canvas->fillRoundRect(x + 5, y + 10, 3, 10, 1, 0x07FF);
                canvas->fillRoundRect(x + 14, y + 10, 3, 10, 1, 0x07FF);
                break;

            case EMOJI_BANANA: // 🍌 Banana
                canvas->fillCircle(x + 11, y + 11, 8, 0xFFE0);
                canvas->fillCircle(x + 14, y + 8, 7, 0x0000);
                canvas->fillRect(x + 4, y + 4, 3, 3, 0x5180);
                break;

            case EMOJI_SKULL: // 💀 Skull
                canvas->fillRoundRect(x + 5, y + 4, 12, 11, 4, 0xFFFF);
                canvas->fillRect(x + 7, y + 14, 8, 5, 0xFFFF);
                canvas->fillCircle(x + 8, y + 9, 2, 0x0000);
                canvas->fillCircle(x + 14, y + 9, 2, 0x0000);
                canvas->drawFastVLine(x + 9, y + 15, 4, 0x0000);
                canvas->drawFastVLine(x + 11, y + 15, 4, 0x0000);
                canvas->drawFastVLine(x + 13, y + 15, 4, 0x0000);
                break;

            case EMOJI_ROCKET: // 🚀 Rocket
                canvas->fillTriangle(x + 16, y + 2, x + 6, y + 12, x + 18, y + 18, 0xFFFF);
                canvas->fillTriangle(x + 16, y + 2, x + 12, y + 6, x + 18, y + 8, 0xF800);
                canvas->fillCircle(x + 12, y + 10, 2, 0x07FF);
                canvas->fillTriangle(x + 4, y + 14, x + 8, y + 16, x + 2, y + 20, 0xFD20);
                break;

            default:
                break;
        }
    }
};
