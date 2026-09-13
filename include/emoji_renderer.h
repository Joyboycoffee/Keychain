#pragma once
#include <LovyanGFX.hpp>

enum EmojiType {
    EMOJI_NONE = 0,
    EMOJI_COFFEE,
    EMOJI_HEART,
    EMOJI_HEART_SPARKLE,
    EMOJI_FIRE,
    EMOJI_ZAP,
    EMOJI_STAR,
    EMOJI_SPARKLES,
    EMOJI_ROCKET,
    EMOJI_CROWN,
    EMOJI_DIAMOND,
    EMOJI_ROBOT,
    EMOJI_ALIEN,
    EMOJI_GAME,
    EMOJI_MUSIC,
    EMOJI_PIZZA,
    EMOJI_BURGER,
    EMOJI_CAT,
    EMOJI_DOG,
    EMOJI_BUNNY,
    EMOJI_BANANA,
    EMOJI_SKULL,
    EMOJI_CRY,
    EMOJI_COOL,
    EMOJI_DEVIL,
    EMOJI_POOP,
    EMOJI_100,
    EMOJI_THUMBSUP,
    EMOJI_MOON,
    EMOJI_FLOWER,
    EMOJI_BEAR,
    EMOJI_TADA,
    EMOJI_EYES
};

class EmojiRenderer {
public:
    static void drawEmoji(LGFX_Sprite* canvas, EmojiType type, int x, int y) {
        if (!canvas) return;

        switch (type) {
            case EMOJI_COFFEE: // ☕ Coffee Cup
                canvas->fillRoundRect(x + 2, y + 6, 16, 14, 3, 0xD440);
                canvas->fillRect(x + 3, y + 7, 14, 3, 0x5180);
                canvas->drawRoundRect(x + 16, y + 8, 6, 10, 2, 0xD440);
                canvas->drawFastVLine(x + 5, y + 1, 3, 0xFFFF);
                canvas->drawFastVLine(x + 9, y + 0, 4, 0xFFFF);
                canvas->drawFastVLine(x + 13, y + 2, 3, 0xFFFF);
                break;

            case EMOJI_HEART: // ❤️ Red Heart
                canvas->fillCircle(x + 6, y + 7, 5, 0xF800);
                canvas->fillCircle(x + 16, y + 7, 5, 0xF800);
                canvas->fillTriangle(x + 1, y + 8, x + 21, y + 8, x + 11, y + 20, 0xF800);
                canvas->fillCircle(x + 5, y + 6, 2, 0xFBE4);
                break;

            case EMOJI_HEART_SPARKLE: // 💖 Pink Sparkle Heart
                canvas->fillCircle(x + 6, y + 7, 5, 0xF81F);
                canvas->fillCircle(x + 16, y + 7, 5, 0xF81F);
                canvas->fillTriangle(x + 1, y + 8, x + 21, y + 8, x + 11, y + 20, 0xF81F);
                canvas->fillCircle(x + 18, y + 4, 2, 0xFFFF);
                canvas->fillCircle(x + 4, y + 16, 1, 0xFFE0);
                break;

            case EMOJI_FIRE: // 🔥 Flame
                canvas->fillTriangle(x + 11, y + 1, x + 3, y + 21, x + 19, y + 21, 0xF800);
                canvas->fillTriangle(x + 11, y + 6, x + 5, y + 21, x + 17, y + 21, 0xFD20);
                canvas->fillTriangle(x + 11, y + 11, x + 7, y + 21, x + 15, y + 21, 0xFFE0);
                break;

            case EMOJI_ZAP: // ⚡ Lightning Bolt
                canvas->fillTriangle(x + 13, y + 1, x + 5, y + 11, x + 12, y + 11, 0xFFE0);
                canvas->fillTriangle(x + 11, y + 9, x + 18, y + 9, x + 9, y + 21, 0xFFE0);
                canvas->fillCircle(x + 11, y + 10, 2, 0xFFFF);
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

            case EMOJI_ROCKET: // 🚀 Rocket
                canvas->fillTriangle(x + 16, y + 2, x + 6, y + 12, x + 18, y + 18, 0xFFFF);
                canvas->fillTriangle(x + 16, y + 2, x + 12, y + 6, x + 18, y + 8, 0xF800);
                canvas->fillCircle(x + 12, y + 10, 2, 0x07FF);
                canvas->fillTriangle(x + 4, y + 14, x + 8, y + 16, x + 2, y + 20, 0xFD20);
                break;

            case EMOJI_CROWN: // 👑 Gold Crown
                canvas->fillRect(x + 3, y + 14, 16, 5, 0xFFE0);
                canvas->fillTriangle(x + 3, y + 14, x + 5, y + 6, x + 9, y + 14, 0xFFE0);
                canvas->fillTriangle(x + 8, y + 14, x + 11, y + 4, x + 14, y + 14, 0xFFE0);
                canvas->fillTriangle(x + 13, y + 14, x + 17, y + 6, x + 19, y + 14, 0xFFE0);
                canvas->fillCircle(x + 5, y + 5, 2, 0xF800);
                canvas->fillCircle(x + 11, y + 3, 2, 0x07FF);
                canvas->fillCircle(x + 17, y + 5, 2, 0xF800);
                break;

            case EMOJI_DIAMOND: // 💎 Blue Diamond
                canvas->fillTriangle(x + 5, y + 6, x + 17, y + 6, x + 11, y + 19, 0x07FF);
                canvas->fillTriangle(x + 5, y + 6, x + 17, y + 6, x + 11, y + 2, 0x07FF);
                canvas->drawTriangle(x + 5, y + 6, x + 17, y + 6, x + 11, y + 19, 0xFFFF);
                canvas->fillCircle(x + 11, y + 6, 2, 0xFFFF);
                break;

            case EMOJI_ROBOT: // 🤖 Robot Face
                canvas->fillRoundRect(x + 4, y + 6, 14, 13, 3, 0x07FF);
                canvas->fillCircle(x + 8, y + 11, 2, 0xFFFF);
                canvas->fillCircle(x + 14, y + 11, 2, 0xFFFF);
                canvas->drawFastHLine(x + 8, y + 15, 6, 0x0000);
                canvas->drawFastVLine(x + 11, y + 2, 4, 0x07FF);
                canvas->fillCircle(x + 11, y + 2, 2, 0xF800);
                break;

            case EMOJI_ALIEN: // 👾 Alien Space Invader
                canvas->fillRoundRect(x + 3, y + 5, 16, 12, 2, 0xF81F);
                canvas->fillCircle(x + 7, y + 9, 2, 0x0000);
                canvas->fillCircle(x + 15, y + 9, 2, 0x0000);
                canvas->fillRect(x + 7, y + 17, 3, 3, 0xF81F);
                canvas->fillRect(x + 12, y + 17, 3, 3, 0xF81F);
                canvas->fillRect(x + 1, y + 8, 3, 4, 0xF81F);
                canvas->fillRect(x + 18, y + 8, 3, 4, 0xF81F);
                break;

            case EMOJI_GAME: // 🎮 Gamepad
                canvas->fillRoundRect(x + 3, y + 6, 16, 11, 4, 0x4208);
                canvas->drawFastHLine(x + 6, y + 11, 4, 0x07FF);
                canvas->drawFastVLine(x + 7, y + 9, 4, 0x07FF);
                canvas->fillCircle(x + 14, y + 10, 1, 0xF800);
                canvas->fillCircle(x + 16, y + 12, 1, 0xFFE0);
                break;

            case EMOJI_MUSIC: // 🎵 Musical Notes
                canvas->fillCircle(x + 6, y + 16, 3, 0x07E0);
                canvas->fillCircle(x + 15, y + 13, 3, 0x07E0);
                canvas->drawFastVLine(x + 8, y + 4, 12, 0x07E0);
                canvas->drawFastVLine(x + 17, y + 2, 11, 0x07E0);
                canvas->fillRect(x + 8, y + 2, 10, 3, 0x07E0);
                break;

            case EMOJI_PIZZA: // 🍕 Pizza Slice
                canvas->fillTriangle(x + 3, y + 4, x + 19, y + 4, x + 11, y + 20, 0xFD20);
                canvas->fillRect(x + 2, y + 3, 18, 3, 0xD440);
                canvas->fillCircle(x + 10, y + 9, 2, 0xF800);
                canvas->fillCircle(x + 7, y + 13, 1, 0xF800);
                canvas->fillCircle(x + 14, y + 13, 1, 0xF800);
                break;

            case EMOJI_BURGER: // 🍔 Burger
                canvas->fillRoundRect(x + 3, y + 4, 16, 6, 3, 0xFD20); // Top bun
                canvas->fillRect(x + 2, y + 10, 18, 2, 0x07E0);       // Lettuce
                canvas->fillRect(x + 3, y + 12, 16, 3, 0x8200);       // Patty
                canvas->fillRoundRect(x + 3, y + 15, 16, 4, 2, 0xFD20); // Bottom bun
                break;

            case EMOJI_CAT: // 🐱 Cat Face
                canvas->fillCircle(x + 11, y + 12, 8, 0xFD20);
                canvas->fillTriangle(x + 3, y + 7, x + 7, y + 1, x + 9, y + 7, 0xFD20);
                canvas->fillTriangle(x + 13, y + 7, x + 15, y + 1, x + 19, y + 7, 0xFD20);
                canvas->fillCircle(x + 8, y + 11, 1, 0x0000);
                canvas->fillCircle(x + 14, y + 11, 1, 0x0000);
                canvas->fillTriangle(x + 10, y + 14, x + 12, y + 14, x + 11, y + 16, 0xF81F);
                break;

            case EMOJI_DOG: // 🐶 Dog Face
                canvas->fillCircle(x + 11, y + 12, 8, 0xD545);
                canvas->fillRoundRect(x + 1, y + 7, 4, 9, 2, 0x8200);  // Ear L
                canvas->fillRoundRect(x + 17, y + 7, 4, 9, 2, 0x8200); // Ear R
                canvas->fillCircle(x + 8, y + 11, 1, 0x0000);
                canvas->fillCircle(x + 14, y + 11, 1, 0x0000);
                canvas->fillCircle(x + 11, y + 14, 2, 0x0000);
                break;

            case EMOJI_BUNNY: // 🐰 Bunny Face
                canvas->fillCircle(x + 11, y + 13, 7, 0xFFFF);
                canvas->fillRoundRect(x + 6, y + 1, 3, 9, 2, 0xFFFF);
                canvas->fillRoundRect(x + 13, y + 1, 3, 9, 2, 0xFFFF);
                canvas->fillRoundRect(x + 7, y + 3, 1, 6, 1, 0xF81F);
                canvas->fillRoundRect(x + 14, y + 3, 1, 6, 1, 0xF81F);
                canvas->fillCircle(x + 8, y + 12, 1, 0x0000);
                canvas->fillCircle(x + 14, y + 12, 1, 0x0000);
                canvas->fillCircle(x + 11, y + 15, 1, 0xF81F);
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

            case EMOJI_CRY: // 😭 Crying Face
                canvas->fillCircle(x + 11, y + 11, 10, 0xFFE0);
                canvas->drawFastHLine(x + 5, y + 9, 4, 0x0000);
                canvas->drawFastHLine(x + 13, y + 9, 4, 0x0000);
                canvas->fillRoundRect(x + 8, y + 14, 6, 4, 1, 0x0000);
                canvas->fillRoundRect(x + 5, y + 10, 3, 10, 1, 0x07FF);
                canvas->fillRoundRect(x + 14, y + 10, 3, 10, 1, 0x07FF);
                break;

            case EMOJI_COOL: // 😎 Sunglasses Face
                canvas->fillCircle(x + 11, y + 11, 10, 0xFFE0);
                canvas->fillRoundRect(x + 4, y + 7, 6, 6, 2, 0x0000);
                canvas->fillRoundRect(x + 12, y + 7, 6, 6, 2, 0x0000);
                canvas->drawFastHLine(x + 9, y + 8, 4, 0x0000);
                canvas->drawFastHLine(x + 8, y + 16, 6, 0x0000);
                break;

            case EMOJI_DEVIL: // 😈 Purple Devil
                canvas->fillCircle(x + 11, y + 12, 8, 0x981F);
                canvas->fillTriangle(x + 4, y + 7, x + 6, y + 1, x + 8, y + 6, 0x981F);
                canvas->fillTriangle(x + 14, y + 6, x + 16, y + 1, x + 18, y + 7, 0x981F);
                canvas->fillCircle(x + 8, y + 11, 1, 0xFFFF);
                canvas->fillCircle(x + 14, y + 11, 1, 0xFFFF);
                canvas->drawFastHLine(x + 8, y + 15, 6, 0x0000);
                break;

            case EMOJI_POOP: // 💩 Poop
                canvas->fillRoundRect(x + 4, y + 6, 14, 13, 5, 0x8200);
                canvas->fillCircle(x + 11, y + 4, 4, 0x8200);
                canvas->fillCircle(x + 8, y + 10, 2, 0xFFFF);
                canvas->fillCircle(x + 14, y + 10, 2, 0xFFFF);
                canvas->fillCircle(x + 8, y + 10, 1, 0x0000);
                canvas->fillCircle(x + 14, y + 10, 1, 0x0000);
                canvas->drawFastHLine(x + 8, y + 14, 6, 0x0000);
                break;

            case EMOJI_100: // 💯 100 Points
                canvas->setTextColor(0xF800, 0x0000);
                canvas->setTextSize(2);
                canvas->drawString("100", x + 1, y + 2);
                canvas->drawFastHLine(x + 2, y + 18, 18, 0xF800);
                canvas->drawFastHLine(x + 2, y + 20, 18, 0xF800);
                break;

            case EMOJI_THUMBSUP: // 👍 Thumbs Up
                canvas->fillRoundRect(x + 4, y + 10, 12, 9, 3, 0xFD20);
                canvas->fillRoundRect(x + 4, y + 3, 5, 10, 2, 0xFD20);
                break;

            case EMOJI_MOON: // 🌙 Crescent Moon
                canvas->fillCircle(x + 11, y + 11, 9, 0xFFE0);
                canvas->fillCircle(x + 16, y + 9, 8, 0x0000);
                break;

            case EMOJI_FLOWER: // 🌸 Cherry Blossom / Flower
                canvas->fillCircle(x + 11, y + 6, 4, 0xF81F);
                canvas->fillCircle(x + 6, y + 11, 4, 0xF81F);
                canvas->fillCircle(x + 16, y + 11, 4, 0xF81F);
                canvas->fillCircle(x + 8, y + 16, 4, 0xF81F);
                canvas->fillCircle(x + 14, y + 16, 4, 0xF81F);
                canvas->fillCircle(x + 11, y + 11, 3, 0xFFE0);
                break;

            case EMOJI_BEAR: // 🧸 Teddy Bear
                canvas->fillCircle(x + 11, y + 13, 7, 0x8200);
                canvas->fillCircle(x + 5, y + 7, 3, 0x8200);
                canvas->fillCircle(x + 17, y + 7, 3, 0x8200);
                canvas->fillCircle(x + 9, y + 11, 1, 0x0000);
                canvas->fillCircle(x + 13, y + 11, 1, 0x0000);
                canvas->fillCircle(x + 11, y + 14, 2, 0x5180);
                break;

            case EMOJI_TADA: // 🎉 Party Popper
                canvas->fillTriangle(x + 2, y + 18, x + 14, y + 18, x + 8, y + 8, 0xFD20);
                canvas->fillCircle(x + 6, y + 4, 2, 0xF81F);
                canvas->fillCircle(x + 14, y + 4, 2, 0x07FF);
                canvas->fillCircle(x + 18, y + 10, 2, 0x07E0);
                canvas->fillCircle(x + 12, y + 10, 2, 0xFFE0);
                break;

            case EMOJI_EYES: // 👀 Eyes
                canvas->fillCircle(x + 6, y + 11, 5, 0xFFFF);
                canvas->fillCircle(x + 16, y + 11, 5, 0xFFFF);
                canvas->fillCircle(x + 7, y + 11, 2, 0x0000);
                canvas->fillCircle(x + 17, y + 11, 2, 0x0000);
                break;

            default:
                break;
        }
    }

    static EmojiType resolveCode(const String& code) {
        if (code == ":coffee:") return EMOJI_COFFEE;
        if (code == ":heart:" || code == ":love:") return EMOJI_HEART;
        if (code == ":heart_sparkle:" || code == ":sparkle_heart:") return EMOJI_HEART_SPARKLE;
        if (code == ":fire:" || code == ":flame:") return EMOJI_FIRE;
        if (code == ":zap:" || code == ":lightning:") return EMOJI_ZAP;
        if (code == ":star:") return EMOJI_STAR;
        if (code == ":sparkles:" || code == ":shine:") return EMOJI_SPARKLES;
        if (code == ":rocket:") return EMOJI_ROCKET;
        if (code == ":crown:" || code == ":king:") return EMOJI_CROWN;
        if (code == ":diamond:" || code == ":gem:") return EMOJI_DIAMOND;
        if (code == ":robot:" || code == ":bot:") return EMOJI_ROBOT;
        if (code == ":alien:" || code == ":invader:") return EMOJI_ALIEN;
        if (code == ":game:" || code == ":gamepad:") return EMOJI_GAME;
        if (code == ":music:" || code == ":note:") return EMOJI_MUSIC;
        if (code == ":pizza:") return EMOJI_PIZZA;
        if (code == ":burger:") return EMOJI_BURGER;
        if (code == ":cat:") return EMOJI_CAT;
        if (code == ":dog:") return EMOJI_DOG;
        if (code == ":bunny:" || code == ":rabbit:") return EMOJI_BUNNY;
        if (code == ":banana:") return EMOJI_BANANA;
        if (code == ":skull:") return EMOJI_SKULL;
        if (code == ":cry:" || code == ":sob:") return EMOJI_CRY;
        if (code == ":cool:" || code == ":sunglasses:") return EMOJI_COOL;
        if (code == ":devil:") return EMOJI_DEVIL;
        if (code == ":poop:") return EMOJI_POOP;
        if (code == ":100:") return EMOJI_100;
        if (code == ":thumbsup:" || code == ":like:") return EMOJI_THUMBSUP;
        if (code == ":moon:") return EMOJI_MOON;
        if (code == ":flower:" || code == ":sakura:") return EMOJI_FLOWER;
        if (code == ":bear:" || code == ":teddy:") return EMOJI_BEAR;
        if (code == ":tada:" || code == ":party:") return EMOJI_TADA;
        if (code == ":eyes:" || code == ":look:") return EMOJI_EYES;
        return EMOJI_NONE;
    }

    static int renderTextWithEmojis(LGFX_Sprite* canvas, const String& text, int startX, int y, int textSize = 3, uint16_t textColor = 0xFFFF, uint16_t bg = 0x0000) {
        if (!canvas) return startX;
        int curX = startX;
        int len = text.length();
        int charWidth = 6 * textSize;

        canvas->setTextColor(textColor, bg);
        canvas->setTextSize(textSize);

        for (int i = 0; i < len; ) {
            // Check for shortcode :name:
            if (text[i] == ':' && i + 1 < len) {
                int nextColon = text.indexOf(':', i + 1);
                if (nextColon != -1 && (nextColon - i) <= 16) {
                    String code = text.substring(i, nextColon + 1);
                    EmojiType et = resolveCode(code);
                    if (et != EMOJI_NONE) {
                        drawEmoji(canvas, et, curX, y);
                        curX += 26;
                        i = nextColon + 1;
                        continue;
                    }
                }
            }

            // Check for multi-byte UTF-8 emoji
            uint8_t c = (uint8_t)text[i];
            if ((c & 0x80) != 0) {
                // Determine UTF-8 sequence length
                int seqLen = 1;
                if ((c & 0xE0) == 0xC0) seqLen = 2;
                else if ((c & 0xF0) == 0xE0) seqLen = 3;
                else if ((c & 0xF8) == 0xF0) seqLen = 4;

                if (i + seqLen <= len) {
                    String utfChar = text.substring(i, i + seqLen);
                    // Check known common UTF-8 emoji sequences
                    EmojiType uEt = EMOJI_NONE;
                    if (utfChar == "☕") uEt = EMOJI_COFFEE;
                    else if (utfChar == "❤️" || utfChar == "❤") uEt = EMOJI_HEART;
                    else if (utfChar == "💖" || utfChar == "💗" || utfChar == "💕") uEt = EMOJI_HEART_SPARKLE;
                    else if (utfChar == "🔥") uEt = EMOJI_FIRE;
                    else if (utfChar == "⚡") uEt = EMOJI_ZAP;
                    else if (utfChar == "⭐" || utfChar == "🌟") uEt = EMOJI_STAR;
                    else if (utfChar == "✨") uEt = EMOJI_SPARKLES;
                    else if (utfChar == "🚀") uEt = EMOJI_ROCKET;
                    else if (utfChar == "👑") uEt = EMOJI_CROWN;
                    else if (utfChar == "💎") uEt = EMOJI_DIAMOND;
                    else if (utfChar == "🤖") uEt = EMOJI_ROBOT;
                    else if (utfChar == "👾") uEt = EMOJI_ALIEN;
                    else if (utfChar == "🎮") uEt = EMOJI_GAME;
                    else if (utfChar == "🎵" || utfChar == "🎶") uEt = EMOJI_MUSIC;
                    else if (utfChar == "🍕") uEt = EMOJI_PIZZA;
                    else if (utfChar == "🍔") uEt = EMOJI_BURGER;
                    else if (utfChar == "🐱" || utfChar == "😺" || utfChar == "😸") uEt = EMOJI_CAT;
                    else if (utfChar == "🐶" || utfChar == "🐕") uEt = EMOJI_DOG;
                    else if (utfChar == "🐰" || utfChar == "🐇") uEt = EMOJI_BUNNY;
                    else if (utfChar == "🍌") uEt = EMOJI_BANANA;
                    else if (utfChar == "💀" || utfChar == "☠️") uEt = EMOJI_SKULL;
                    else if (utfChar == "😭" || utfChar == "😢") uEt = EMOJI_CRY;
                    else if (utfChar == "😎") uEt = EMOJI_COOL;
                    else if (utfChar == "😈" || utfChar == "👿") uEt = EMOJI_DEVIL;
                    else if (utfChar == "💩") uEt = EMOJI_POOP;
                    else if (utfChar == "💯") uEt = EMOJI_100;
                    else if (utfChar == "👍") uEt = EMOJI_THUMBSUP;
                    else if (utfChar == "🌙" || utfChar == "🌕") uEt = EMOJI_MOON;
                    else if (utfChar == "🌸" || utfChar == "🌺") uEt = EMOJI_FLOWER;
                    else if (utfChar == "🧸") uEt = EMOJI_BEAR;
                    else if (utfChar == "🎉" || utfChar == "🎊") uEt = EMOJI_TADA;
                    else if (utfChar == "👀") uEt = EMOJI_EYES;

                    if (uEt != EMOJI_NONE) {
                        drawEmoji(canvas, uEt, curX, y);
                        curX += 26;
                        i += seqLen;
                        continue;
                    }
                }
                // Skip unhandled UTF-8 character bytes
                i += seqLen;
                continue;
            }

            char buf[2] = { text[i], '\0' };
            canvas->drawString(buf, curX, y + 2);
            curX += charWidth;
            i++;
        }
        return curX;
    }
};
