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
private:
    inline static int sc(int val, float s) {
        int res = (int)(val * s + 0.5f);
        return (res < 1 && val > 0) ? 1 : res;
    }

    inline static void fCircle(LGFX_Sprite* c, int x, int y, int cx, int cy, int r, uint16_t col, float s) {
        int rad = sc(r, s);
        if (rad < 1) rad = 1;
        c->fillCircle(x + sc(cx, s), y + sc(cy, s), rad, col);
    }

    inline static void fRect(LGFX_Sprite* c, int x, int y, int rx, int ry, int w, int h, uint16_t col, float s) {
        c->fillRect(x + sc(rx, s), y + sc(ry, s), sc(w, s), sc(h, s), col);
    }

    inline static void fRRect(LGFX_Sprite* c, int x, int y, int rx, int ry, int w, int h, int r, uint16_t col, float s) {
        int rw = sc(w, s);
        int rh = sc(h, s);
        int rad = sc(r, s);
        if (rad > rw / 2) rad = rw / 2;
        if (rad > rh / 2) rad = rh / 2;
        if (rad < 1) rad = 1;
        c->fillRoundRect(x + sc(rx, s), y + sc(ry, s), rw, rh, rad, col);
    }

    inline static void dRRect(LGFX_Sprite* c, int x, int y, int rx, int ry, int w, int h, int r, uint16_t col, float s) {
        int rw = sc(w, s);
        int rh = sc(h, s);
        int rad = sc(r, s);
        if (rad > rw / 2) rad = rw / 2;
        if (rad > rh / 2) rad = rh / 2;
        if (rad < 1) rad = 1;
        c->drawRoundRect(x + sc(rx, s), y + sc(ry, s), rw, rh, rad, col);
    }

    inline static void fTri(LGFX_Sprite* c, int x, int y, int x0, int y0, int x1, int y1, int x2, int y2, uint16_t col, float s) {
        c->fillTriangle(x + sc(x0, s), y + sc(y0, s), x + sc(x1, s), y + sc(y1, s), x + sc(x2, s), y + sc(y2, s), col);
    }

    inline static void dTri(LGFX_Sprite* c, int x, int y, int x0, int y0, int x1, int y1, int x2, int y2, uint16_t col, float s) {
        c->drawTriangle(x + sc(x0, s), y + sc(y0, s), x + sc(x1, s), y + sc(y1, s), x + sc(x2, s), y + sc(y2, s), col);
    }

    inline static void fHLine(LGFX_Sprite* c, int x, int y, int lx, int ly, int w, uint16_t col, float s) {
        c->drawFastHLine(x + sc(lx, s), y + sc(ly, s), sc(w, s), col);
        if (s >= 2.0f) {
            c->drawFastHLine(x + sc(lx, s), y + sc(ly, s) + 1, sc(w, s), col);
        }
    }

    inline static void fVLine(LGFX_Sprite* c, int x, int y, int lx, int ly, int h, uint16_t col, float s) {
        c->drawFastVLine(x + sc(lx, s), y + sc(ly, s), sc(h, s), col);
        if (s >= 2.0f) {
            c->drawFastVLine(x + sc(lx, s) + 1, y + sc(ly, s), sc(h, s), col);
        }
    }

public:
    static void drawEmoji(LGFX_Sprite* canvas, EmojiType type, int x, int y, float s = 1.0f) {
        if (!canvas) return;

        switch (type) {
            case EMOJI_COFFEE: // ☕ Coffee Cup
                fRRect(canvas, x, y, 2, 6, 16, 14, 3, 0xD440, s);
                fRect(canvas, x, y, 3, 7, 14, 3, 0x5180, s);
                dRRect(canvas, x, y, 16, 8, 6, 10, 2, 0xD440, s);
                fVLine(canvas, x, y, 5, 1, 3, 0xFFFF, s);
                fVLine(canvas, x, y, 9, 0, 4, 0xFFFF, s);
                fVLine(canvas, x, y, 13, 2, 3, 0xFFFF, s);
                break;

            case EMOJI_HEART: // ❤️ Red Heart
                fCircle(canvas, x, y, 6, 7, 5, 0xF800, s);
                fCircle(canvas, x, y, 16, 7, 5, 0xF800, s);
                fTri(canvas, x, y, 1, 8, 21, 8, 11, 20, 0xF800, s);
                fCircle(canvas, x, y, 5, 6, 2, 0xFBE4, s);
                break;

            case EMOJI_HEART_SPARKLE: // 💖 Pink Sparkle Heart
                fCircle(canvas, x, y, 6, 7, 5, 0xF81F, s);
                fCircle(canvas, x, y, 16, 7, 5, 0xF81F, s);
                fTri(canvas, x, y, 1, 8, 21, 8, 11, 20, 0xF81F, s);
                fCircle(canvas, x, y, 18, 4, 2, 0xFFFF, s);
                fCircle(canvas, x, y, 4, 16, 1, 0xFFE0, s);
                break;

            case EMOJI_FIRE: // 🔥 Flame
                fTri(canvas, x, y, 11, 1, 3, 21, 19, 21, 0xF800, s);
                fTri(canvas, x, y, 11, 6, 5, 21, 17, 21, 0xFD20, s);
                fTri(canvas, x, y, 11, 11, 7, 21, 15, 21, 0xFFE0, s);
                break;

            case EMOJI_ZAP: // ⚡ Lightning Bolt
                fTri(canvas, x, y, 13, 1, 5, 11, 12, 11, 0xFFE0, s);
                fTri(canvas, x, y, 11, 9, 18, 9, 9, 21, 0xFFE0, s);
                fCircle(canvas, x, y, 11, 10, 2, 0xFFFF, s);
                break;

            case EMOJI_STAR: // ⭐ Gold Star
                fTri(canvas, x, y, 11, 1, 2, 19, 20, 19, 0xFFE0, s);
                fTri(canvas, x, y, 11, 19, 2, 7, 20, 7, 0xFFE0, s);
                fCircle(canvas, x, y, 11, 11, 4, 0xFFFF, s);
                break;

            case EMOJI_SPARKLES: // ✨ Sparkles
                fCircle(canvas, x, y, 8, 8, 3, 0x07FF, s);
                fHLine(canvas, x, y, 2, 8, 13, 0x07FF, s);
                fVLine(canvas, x, y, 8, 2, 13, 0x07FF, s);
                fCircle(canvas, x, y, 17, 16, 2, 0xFFE0, s);
                fHLine(canvas, x, y, 13, 16, 9, 0xFFE0, s);
                fVLine(canvas, x, y, 17, 12, 9, 0xFFE0, s);
                break;

            case EMOJI_ROCKET: // 🚀 Rocket
                fTri(canvas, x, y, 16, 2, 6, 12, 18, 18, 0xFFFF, s);
                fTri(canvas, x, y, 16, 2, 12, 6, 18, 8, 0xF800, s);
                fCircle(canvas, x, y, 12, 10, 2, 0x07FF, s);
                fTri(canvas, x, y, 4, 14, 8, 16, 2, 20, 0xFD20, s);
                break;

            case EMOJI_CROWN: // 👑 Gold Crown
                fRect(canvas, x, y, 3, 14, 16, 5, 0xFFE0, s);
                fTri(canvas, x, y, 3, 14, 5, 6, 9, 14, 0xFFE0, s);
                fTri(canvas, x, y, 8, 14, 11, 4, 14, 14, 0xFFE0, s);
                fTri(canvas, x, y, 13, 14, 17, 6, 19, 14, 0xFFE0, s);
                fCircle(canvas, x, y, 5, 5, 2, 0xF800, s);
                fCircle(canvas, x, y, 11, 3, 2, 0x07FF, s);
                fCircle(canvas, x, y, 17, 5, 2, 0xF800, s);
                break;

            case EMOJI_DIAMOND: // 💎 Blue Diamond
                fTri(canvas, x, y, 5, 6, 17, 6, 11, 19, 0x07FF, s);
                fTri(canvas, x, y, 5, 6, 17, 6, 11, 2, 0x07FF, s);
                dTri(canvas, x, y, 5, 6, 17, 6, 11, 19, 0xFFFF, s);
                fCircle(canvas, x, y, 11, 6, 2, 0xFFFF, s);
                break;

            case EMOJI_ROBOT: // 🤖 Robot Face
                fRRect(canvas, x, y, 4, 6, 14, 13, 3, 0x07FF, s);
                fCircle(canvas, x, y, 8, 11, 2, 0xFFFF, s);
                fCircle(canvas, x, y, 14, 11, 2, 0xFFFF, s);
                fHLine(canvas, x, y, 8, 15, 6, 0x0000, s);
                fVLine(canvas, x, y, 11, 2, 4, 0x07FF, s);
                fCircle(canvas, x, y, 11, 2, 2, 0xF800, s);
                break;

            case EMOJI_ALIEN: // 👾 Alien Space Invader
                fRRect(canvas, x, y, 3, 5, 16, 12, 2, 0xF81F, s);
                fCircle(canvas, x, y, 7, 9, 2, 0x0000, s);
                fCircle(canvas, x, y, 15, 9, 2, 0x0000, s);
                fRect(canvas, x, y, 7, 17, 3, 3, 0xF81F, s);
                fRect(canvas, x, y, 12, 17, 3, 3, 0xF81F, s);
                fRect(canvas, x, y, 1, 8, 3, 4, 0xF81F, s);
                fRect(canvas, x, y, 18, 8, 3, 4, 0xF81F, s);
                break;

            case EMOJI_GAME: // 🎮 Gamepad
                fRRect(canvas, x, y, 3, 6, 16, 11, 4, 0x4208, s);
                fHLine(canvas, x, y, 6, 11, 4, 0x07FF, s);
                fVLine(canvas, x, y, 7, 9, 4, 0x07FF, s);
                fCircle(canvas, x, y, 14, 10, 1, 0xF800, s);
                fCircle(canvas, x, y, 16, 12, 1, 0xFFE0, s);
                break;

            case EMOJI_MUSIC: // 🎵 Musical Notes
                fCircle(canvas, x, y, 6, 16, 3, 0x07E0, s);
                fCircle(canvas, x, y, 15, 13, 3, 0x07E0, s);
                fVLine(canvas, x, y, 8, 4, 12, 0x07E0, s);
                fVLine(canvas, x, y, 17, 2, 11, 0x07E0, s);
                fRect(canvas, x, y, 8, 2, 10, 3, 0x07E0, s);
                break;

            case EMOJI_PIZZA: // 🍕 Pizza Slice
                fTri(canvas, x, y, 3, 4, 19, 4, 11, 20, 0xFD20, s);
                fRect(canvas, x, y, 2, 3, 18, 3, 0xD440, s);
                fCircle(canvas, x, y, 10, 9, 2, 0xF800, s);
                fCircle(canvas, x, y, 7, 13, 1, 0xF800, s);
                fCircle(canvas, x, y, 14, 13, 1, 0xF800, s);
                break;

            case EMOJI_BURGER: // 🍔 Burger
                fRRect(canvas, x, y, 3, 4, 16, 6, 3, 0xFD20, s);
                fRect(canvas, x, y, 2, 10, 18, 2, 0x07E0, s);
                fRect(canvas, x, y, 3, 12, 16, 3, 0x8200, s);
                fRRect(canvas, x, y, 3, 15, 16, 4, 2, 0xFD20, s);
                break;

            case EMOJI_CAT: // 🐱 Cat Face
                fCircle(canvas, x, y, 11, 12, 8, 0xFD20, s);
                fTri(canvas, x, y, 3, 7, 7, 1, 9, 7, 0xFD20, s);
                fTri(canvas, x, y, 13, 7, 15, 1, 19, 7, 0xFD20, s);
                fCircle(canvas, x, y, 8, 11, 1, 0x0000, s);
                fCircle(canvas, x, y, 14, 11, 1, 0x0000, s);
                fTri(canvas, x, y, 10, 14, 12, 14, 11, 16, 0xF81F, s);
                break;

            case EMOJI_DOG: // 🐶 Dog Face
                fCircle(canvas, x, y, 11, 12, 8, 0xD545, s);
                fRRect(canvas, x, y, 1, 7, 4, 9, 2, 0x8200, s);
                fRRect(canvas, x, y, 17, 7, 4, 9, 2, 0x8200, s);
                fCircle(canvas, x, y, 8, 11, 1, 0x0000, s);
                fCircle(canvas, x, y, 14, 11, 1, 0x0000, s);
                fCircle(canvas, x, y, 11, 14, 2, 0x0000, s);
                break;

            case EMOJI_BUNNY: // 🐰 Bunny Face
                fCircle(canvas, x, y, 11, 13, 7, 0xFFFF, s);
                fRRect(canvas, x, y, 6, 1, 3, 9, 2, 0xFFFF, s);
                fRRect(canvas, x, y, 13, 1, 3, 9, 2, 0xFFFF, s);
                fRRect(canvas, x, y, 7, 3, 1, 6, 1, 0xF81F, s);
                fRRect(canvas, x, y, 14, 3, 1, 6, 1, 0xF81F, s);
                fCircle(canvas, x, y, 8, 12, 1, 0x0000, s);
                fCircle(canvas, x, y, 14, 12, 1, 0x0000, s);
                fCircle(canvas, x, y, 11, 15, 1, 0xF81F, s);
                break;

            case EMOJI_BANANA: // 🍌 Banana
                fCircle(canvas, x, y, 11, 11, 8, 0xFFE0, s);
                fCircle(canvas, x, y, 14, 8, 7, 0x0000, s);
                fRect(canvas, x, y, 4, 4, 3, 3, 0x5180, s);
                break;

            case EMOJI_SKULL: // 💀 Skull
                fRRect(canvas, x, y, 5, 4, 12, 11, 4, 0xFFFF, s);
                fRect(canvas, x, y, 7, 14, 8, 5, 0xFFFF, s);
                fCircle(canvas, x, y, 8, 9, 2, 0x0000, s);
                fCircle(canvas, x, y, 14, 9, 2, 0x0000, s);
                fVLine(canvas, x, y, 9, 15, 4, 0x0000, s);
                fVLine(canvas, x, y, 11, 15, 4, 0x0000, s);
                fVLine(canvas, x, y, 13, 15, 4, 0x0000, s);
                break;

            case EMOJI_CRY: // 😭 Crying Face
                fCircle(canvas, x, y, 11, 11, 10, 0xFFE0, s);
                fHLine(canvas, x, y, 5, 9, 4, 0x0000, s);
                fHLine(canvas, x, y, 13, 9, 4, 0x0000, s);
                fRRect(canvas, x, y, 8, 14, 6, 4, 1, 0x0000, s);
                fRRect(canvas, x, y, 5, 10, 3, 10, 1, 0x07FF, s);
                fRRect(canvas, x, y, 14, 10, 3, 10, 1, 0x07FF, s);
                break;

            case EMOJI_COOL: // 😎 Sunglasses Face
                fCircle(canvas, x, y, 11, 11, 10, 0xFFE0, s);
                fRRect(canvas, x, y, 4, 7, 6, 6, 2, 0x0000, s);
                fRRect(canvas, x, y, 12, 7, 6, 6, 2, 0x0000, s);
                fHLine(canvas, x, y, 9, 8, 4, 0x0000, s);
                fHLine(canvas, x, y, 8, 16, 6, 0x0000, s);
                break;

            case EMOJI_DEVIL: // 😈 Purple Devil
                fCircle(canvas, x, y, 11, 12, 8, 0x981F, s);
                fTri(canvas, x, y, 4, 7, 6, 1, 8, 6, 0x981F, s);
                fTri(canvas, x, y, 14, 6, 16, 1, 18, 7, 0x981F, s);
                fCircle(canvas, x, y, 8, 11, 1, 0xFFFF, s);
                fCircle(canvas, x, y, 14, 11, 1, 0xFFFF, s);
                fHLine(canvas, x, y, 8, 15, 6, 0x0000, s);
                break;

            case EMOJI_POOP: // 💩 Poop
                fRRect(canvas, x, y, 4, 6, 14, 13, 5, 0x8200, s);
                fCircle(canvas, x, y, 11, 4, 4, 0x8200, s);
                fCircle(canvas, x, y, 8, 10, 2, 0xFFFF, s);
                fCircle(canvas, x, y, 14, 10, 2, 0xFFFF, s);
                fCircle(canvas, x, y, 8, 10, 1, 0x0000, s);
                fCircle(canvas, x, y, 14, 10, 1, 0x0000, s);
                fHLine(canvas, x, y, 8, 14, 6, 0x0000, s);
                break;

            case EMOJI_100: // 💯 100 Points
                canvas->setTextColor(0xF800, 0x0000);
                canvas->setTextSize(s >= 2.0f ? (int)(2.0f * s) : (s < 0.7f ? 1 : 2));
                canvas->drawString("100", x + sc(1, s), y + sc(2, s));
                fHLine(canvas, x, y, 2, 18, 18, 0xF800, s);
                fHLine(canvas, x, y, 2, 20, 18, 0xF800, s);
                break;

            case EMOJI_THUMBSUP: // 👍 Thumbs Up
                fRRect(canvas, x, y, 4, 10, 12, 9, 3, 0xFD20, s);
                fRRect(canvas, x, y, 4, 3, 5, 10, 2, 0xFD20, s);
                break;

            case EMOJI_MOON: // 🌙 Crescent Moon
                fCircle(canvas, x, y, 11, 11, 9, 0xFFE0, s);
                fCircle(canvas, x, y, 16, 9, 8, 0x0000, s);
                break;

            case EMOJI_FLOWER: // 🌸 Cherry Blossom / Flower
                fCircle(canvas, x, y, 11, 6, 4, 0xF81F, s);
                fCircle(canvas, x, y, 6, 11, 4, 0xF81F, s);
                fCircle(canvas, x, y, 16, 11, 4, 0xF81F, s);
                fCircle(canvas, x, y, 8, 16, 4, 0xF81F, s);
                fCircle(canvas, x, y, 14, 16, 4, 0xF81F, s);
                fCircle(canvas, x, y, 11, 11, 3, 0xFFE0, s);
                break;

            case EMOJI_BEAR: // 🧸 Teddy Bear
                fCircle(canvas, x, y, 11, 13, 7, 0x8200, s);
                fCircle(canvas, x, y, 5, 7, 3, 0x8200, s);
                fCircle(canvas, x, y, 17, 7, 3, 0x8200, s);
                fCircle(canvas, x, y, 9, 11, 1, 0x0000, s);
                fCircle(canvas, x, y, 13, 11, 1, 0x0000, s);
                fCircle(canvas, x, y, 11, 14, 2, 0x5180, s);
                break;

            case EMOJI_TADA: // 🎉 Party Popper
                fTri(canvas, x, y, 2, 18, 14, 18, 8, 8, 0xFD20, s);
                fCircle(canvas, x, y, 6, 4, 2, 0xF81F, s);
                fCircle(canvas, x, y, 14, 4, 2, 0x07FF, s);
                fCircle(canvas, x, y, 18, 10, 2, 0x07E0, s);
                fCircle(canvas, x, y, 12, 10, 2, 0xFFE0, s);
                break;

            case EMOJI_EYES: // 👀 Eyes
                fCircle(canvas, x, y, 6, 11, 5, 0xFFFF, s);
                fCircle(canvas, x, y, 16, 11, 5, 0xFFFF, s);
                fCircle(canvas, x, y, 7, 11, 2, 0x0000, s);
                fCircle(canvas, x, y, 17, 11, 2, 0x0000, s);
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
        float s = (float)textSize / 3.0f;
        int emojiWidth = (int)(24.0f * s) + (textSize <= 2 ? 2 : (textSize <= 4 ? 4 : (textSize <= 8 ? 6 : 8)));

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
                        drawEmoji(canvas, et, curX, y, s);
                        curX += emojiWidth;
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
                        drawEmoji(canvas, uEt, curX, y, s);
                        curX += emojiWidth;
                        i += seqLen;
                        continue;
                    }
                }
                // Skip unhandled UTF-8 character bytes
                i += seqLen;
                continue;
            }

            char buf[2] = { text[i], '\0' };
            canvas->drawString(buf, curX, y);
            curX += charWidth;
            i++;
        }
        return curX;
    }
};
