#pragma once
#include <Arduino.h>
#include <LovyanGFX.hpp>
#include <AnimatedGIF.h>
#include <LittleFS.h>
#include "display_setup.h"
#include "config.h"

extern LGFX_Sprite canvas;

// File callbacks for AnimatedGIF with LittleFS
static File s_gifFile;

static void* GIFOpenFile(const char *fname, int32_t *pSize) {
    s_gifFile = LittleFS.open(fname, "r");
    if (s_gifFile) {
        *pSize = s_gifFile.size();
        return (void*)&s_gifFile;
    }
    return nullptr;
}

static void GIFCloseFile(void *pHandle) {
    File *f = static_cast<File*>(pHandle);
    if (f != nullptr && *f) {
        f->close();
    }
}

static int32_t GIFReadFile(GIFFILE *pFile, uint8_t *pBuf, int32_t iLen) {
    int32_t iBytesRead = iLen;
    File *f = static_cast<File*>(pFile->fHandle);
    if (!f || !*f) return 0;
    if ((pFile->iSize - pFile->iPos) < iLen)
        iBytesRead = pFile->iSize - pFile->iPos;
    if (iBytesRead <= 0) return 0;
    iBytesRead = (int32_t)f->read(pBuf, iBytesRead);
    pFile->iPos = f->position();
    return iBytesRead;
}

static int32_t GIFSeekFile(GIFFILE *pFile, int32_t iPosition) {
    File *f = static_cast<File*>(pFile->fHandle);
    if (!f || !*f) return 0;
    f->seek(iPosition);
    pFile->iPos = (int32_t)f->position();
    return pFile->iPos;
}

// Global draw state for AnimatedGIF
static int s_gifOffsetX = 0;
static int s_gifOffsetY = 0;
static uint16_t s_activeColorTint = 0xFFFF; // 0xFFFF = Crisp Pure White
static bool s_enableColorTint = false;

// Fast inline color tinting in RGB565 Little Endian
static inline uint16_t fastTintRGB565(uint16_t col, uint16_t tint) {
    if (col == 0x0000) return 0x0000;
    if (col == 0xFFFF && tint == 0xFFFF) return 0xFFFF;

    uint8_t r = (col >> 11) & 0x1F;
    uint8_t g = (col >> 5) & 0x3F;
    uint8_t b = col & 0x1F;
    float lum = ((float)r / 31.0f + (float)g / 63.0f + (float)b / 31.0f) * 0.3333f;

    uint8_t tr = (tint >> 11) & 0x1F;
    uint8_t tg = (tint >> 5) & 0x3F;
    uint8_t tb = tint & 0x1F;

    return (((uint8_t)(tr * lum)) << 11) |
           (((uint8_t)(tg * lum)) << 5)  |
           ((uint8_t)(tb * lum));
}

// High-Speed Direct-to-Framebuffer Line Callback (<0.5ms per frame)
static void GIFDrawCallback(GIFDRAW *pDraw) {
    uint8_t *s = pDraw->pPixels;
    uint16_t *usPalette = pDraw->pPalette;
    int y = pDraw->iY + pDraw->y + s_gifOffsetY;
    if (y < 0 || y >= 240) return;

    uint16_t *dstRow = ((uint16_t*)canvas.getBuffer()) + (y * 240);
    int startX = pDraw->iX + s_gifOffsetX;
    int width = pDraw->iWidth;

    // Handle background restore disposal
    if (pDraw->ucDisposalMethod == 2) {
        for (int x = 0; x < width; x++) {
            if (s[x] == pDraw->ucTransparent)
                s[x] = pDraw->ucBackground;
        }
        pDraw->ucHasTransparency = 0;
    }

    if (pDraw->ucHasTransparency) {
        uint8_t ucTrans = pDraw->ucTransparent;
        for (int x = 0; x < width; x++) {
            int px = startX + x;
            if (px >= 0 && px < 240) {
                uint8_t c = s[x];
                if (c != ucTrans) {
                    uint16_t col = usPalette[c];
                    if (s_enableColorTint) {
                        col = fastTintRGB565(col, s_activeColorTint);
                    }
                    dstRow[px] = col;
                }
            }
        }
    } else {
        for (int x = 0; x < width; x++) {
            int px = startX + x;
            if (px >= 0 && px < 240) {
                uint16_t col = usPalette[s[x]];
                if (s_enableColorTint) {
                    col = fastTintRGB565(col, s_activeColorTint);
                }
                dstRow[px] = col;
            }
        }
    }
}

class DasaiGifPlayer {
public:
    AnimatedGIF gif;
    bool isLoaded = false;
    String currentGifPath = "";
    int currentStyle = 0; // 0=Pure White, 1=Pastel Cyan, 2=Sakura Pink, 3=Mochi Gold, 4=Neon Violet
    int nextFrameDelayMs = 20;
    uint32_t lastFrameTime = 0;
    
    // Playback state
    bool loopContinuous = true;
    int loopCount = 0;
    int maxLoops = 1;
    String returnAfterPath = "/mochi/blank.gif";
    int playbackSpeedMultiplier = 2; // 2x speed for silky smooth 50-60 FPS playback

    DasaiGifPlayer() {
        // Use Little-Endian RGB565 for crisp pixel-perfect colors matching LovyanGFX
        gif.begin(GIF_PALETTE_RGB565_LE);
    }

    void setStyle(int style) {
        currentStyle = (style >= 0 && style < 5) ? style : 0;
        switch (currentStyle) {
            case 0: s_activeColorTint = 0xFFFF; s_enableColorTint = false; break; // Crisp Original White
            case 1: s_activeColorTint = 0x7FFF; s_enableColorTint = true;  break; // Pastel Cyan
            case 2: s_activeColorTint = 0xFDF7; s_enableColorTint = true;  break; // Sakura Pink
            case 3: s_activeColorTint = 0xFFE0; s_enableColorTint = true;  break; // Mochi Gold
            case 4: s_activeColorTint = 0xD69A; s_enableColorTint = true;  break; // Neon Violet
        }
    }

    int cycleStyle() {
        setStyle((currentStyle + 1) % 5);
        return currentStyle;
    }

    bool playGif(const String& path, bool loop = true, int repeatCount = 1) {
        if (!LittleFS.exists(path)) {
            Serial.printf("[MOCHI_GIF] File not found: %s\n", path.c_str());
            return false;
        }

        if (isLoaded) {
            gif.close();
            isLoaded = false;
        }

        if (!gif.open(path.c_str(), GIFOpenFile, GIFCloseFile, GIFReadFile, GIFSeekFile, GIFDrawCallback)) {
            Serial.printf("[MOCHI_GIF] Failed to open GIF: %s\n", path.c_str());
            return false;
        }

        currentGifPath = path;
        isLoaded = true;
        loopContinuous = loop;
        loopCount = 0;
        maxLoops = repeatCount;

        // Auto calculate centering offsets for 240x240 screen
        int gifW = gif.getCanvasWidth();
        int gifH = gif.getCanvasHeight();
        s_gifOffsetX = (240 - gifW) / 2;
        s_gifOffsetY = (240 - gifH) / 2;

        setStyle(currentStyle);
        lastFrameTime = millis();
        nextFrameDelayMs = 0;

        canvas.fillScreen(TFT_BLACK);
        Serial.printf("[MOCHI_GIF] Playing authentic: %s (%dx%d, offsets: %d,%d)\n", path.c_str(), gifW, gifH, s_gifOffsetX, s_gifOffsetY);
        return true;
    }

    void playEmotion(const String& name) {
        String path = "/mochi/" + name + ".gif";
        if (!LittleFS.exists(path)) {
            path = "/" + name + ".gif";
        }
        if (!LittleFS.exists(path)) {
            path = "/mochi/blank.gif";
        }
        playGif(path, (name == "blank"), 1);
    }

    void update() {
        if (!isLoaded) {
            if (LittleFS.exists("/mochi/blank.gif")) {
                playGif("/mochi/blank.gif", true);
            } else if (LittleFS.exists("/blank.gif")) {
                playGif("/blank.gif", true);
            }
            return;
        }

        uint32_t now = millis();
        if (now - lastFrameTime >= (uint32_t)nextFrameDelayMs) {
            lastFrameTime = now;
            int rawDelay = 33;
            int result = gif.playFrame(true, &rawDelay);

            if (result == 0) {
                // End of GIF reached
                loopCount++;
                if (loopContinuous || loopCount < maxLoops) {
                    gif.reset();
                } else {
                    if (currentGifPath != returnAfterPath && LittleFS.exists(returnAfterPath)) {
                        playGif(returnAfterPath, true);
                    } else {
                        gif.reset();
                    }
                }
            }

            // High-speed smooth pacing (boost 8-16 FPS GIF metadata up to 50-60 FPS)
            if (rawDelay <= 0) rawDelay = 33;
            int targetDelay = rawDelay / playbackSpeedMultiplier;
            if (targetDelay < 16) targetDelay = 16; // 60 FPS cap
            if (targetDelay > 45) targetDelay = 45; // prevent sluggish 8 FPS slowdown
            nextFrameDelayMs = targetDelay;
        }
    }
};
