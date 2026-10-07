#pragma once
#include <Arduino.h>
#include <LovyanGFX.hpp>
#include <AnimatedGIF.h>
#include <LittleFS.h>
#include "display_setup.h"
#include "config.h"

// Forward declaration of global canvas
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
static uint16_t s_activeColorTint = 0xFFFF; // 0xFFFF = White, or custom palette
static bool s_enableColorTint = false;

static void GIFDrawCallback(GIFDRAW *pDraw) {
    uint8_t *s;
    uint16_t *usPalette;
    int x, y, iWidth;

    iWidth = pDraw->iWidth;
    if (iWidth + pDraw->iX > 240)
        iWidth = 240 - pDraw->iX;
    usPalette = pDraw->pPalette;
    y = pDraw->iY + pDraw->y; // current line
    if (y >= 240 || pDraw->iX >= 240 || iWidth < 1)
        return;

    int drawY = y + s_gifOffsetY;
    if (drawY < 0 || drawY >= 240) return;

    s = pDraw->pPixels;

    if (pDraw->ucDisposalMethod == 2) { // Restore to background
        for (x = 0; x < iWidth; x++) {
            if (s[x] == pDraw->ucTransparent)
                s[x] = pDraw->ucBackground;
        }
        pDraw->ucHasTransparency = 0;
    }

    if (pDraw->ucHasTransparency) {
        uint8_t *pEnd = s + iWidth;
        int xPos = pDraw->iX + s_gifOffsetX;
        while (s < pEnd) {
            uint8_t c = *s++;
            if (c != pDraw->ucTransparent) {
                if (xPos >= 0 && xPos < 240) {
                    uint16_t col = usPalette[c];
                    if (s_enableColorTint && col != 0x0000) {
                        // Extract brightness and tint
                        uint8_t r = (col >> 11) & 0x1F;
                        uint8_t g = (col >> 5) & 0x3F;
                        uint8_t b = col & 0x1F;
                        float brightness = ((float)r / 31.0f + (float)g / 63.0f + (float)b / 31.0f) / 3.0f;
                        uint8_t tr = (s_activeColorTint >> 11) & 0x1F;
                        uint8_t tg = (s_activeColorTint >> 5) & 0x3F;
                        uint8_t tb = s_activeColorTint & 0x1F;
                        col = (((uint8_t)(tr * brightness)) << 11) |
                              (((uint8_t)(tg * brightness)) << 5) |
                              ((uint8_t)(tb * brightness));
                    }
                    canvas.drawPixel(xPos, drawY, col);
                }
            }
            xPos++;
        }
    } else {
        for (x = 0; x < iWidth; x++) {
            int xPos = pDraw->iX + s_gifOffsetX + x;
            if (xPos >= 0 && xPos < 240) {
                uint16_t col = usPalette[s[x]];
                if (s_enableColorTint && col != 0x0000) {
                    uint8_t r = (col >> 11) & 0x1F;
                    uint8_t g = (col >> 5) & 0x3F;
                    uint8_t b = col & 0x1F;
                    float brightness = ((float)r / 31.0f + (float)g / 63.0f + (float)b / 31.0f) / 3.0f;
                    uint8_t tr = (s_activeColorTint >> 11) & 0x1F;
                    uint8_t tg = (s_activeColorTint >> 5) & 0x3F;
                    uint8_t tb = s_activeColorTint & 0x1F;
                    col = (((uint8_t)(tr * brightness)) << 11) |
                          (((uint8_t)(tg * brightness)) << 5) |
                          ((uint8_t)(tb * brightness));
                }
                canvas.drawPixel(xPos, drawY, col);
            }
        }
    }
}

class DasaiGifPlayer {
public:
    AnimatedGIF gif;
    bool isLoaded = false;
    String currentGifPath = "";
    int currentStyle = 0; // 0=White, 1=Pastel Cyan, 2=Sakura Pink, 3=Mochi Gold, 4=Cyber Violet
    int nextFrameDelayMs = 33;
    uint32_t lastFrameTime = 0;
    
    // Playback state
    bool loopContinuous = true;
    int loopCount = 0;
    int maxLoops = 1;
    String returnAfterPath = "/mochi/blank.gif";

    DasaiGifPlayer() {
        gif.begin(GIF_PALETTE_RGB565_BE);
    }

    void setStyle(int style) {
        currentStyle = (style >= 0 && style < 5) ? style : 0;
        switch (currentStyle) {
            case 0: s_activeColorTint = 0xFFFF; s_enableColorTint = false; break; // Crisp Original White
            case 1: s_activeColorTint = 0x7FFF; s_enableColorTint = true;  break; // Pastel Cyan
            case 2: s_activeColorTint = 0xFDF7; s_enableColorTint = true;  break; // Sakura Pink
            case 3: s_activeColorTint = 0xFFE0; s_enableColorTint = true;  break; // Mochi Gold
            case 4: s_activeColorTint = 0xD69A; s_enableColorTint = true;  break; // Cyber Violet
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
        Serial.printf("[MOCHI_GIF] Playing: %s (%dx%d, offsets: %d,%d)\n", path.c_str(), gifW, gifH, s_gifOffsetX, s_gifOffsetY);
        return true;
    }

    void playEmotion(const String& name) {
        String path = "/mochi/" + name + ".gif";
        if (!path.endsWith(".gif")) path += ".gif";
        if (!LittleFS.exists(path)) {
            path = "/" + name + ".gif";
        }
        playGif(path, false, 1);
    }

    void update() {
        if (!isLoaded) {
            // If nothing is playing, load default idle looking/blinking GIF
            if (LittleFS.exists("/mochi/blank.gif")) {
                playGif("/mochi/blank.gif", true);
            } else if (LittleFS.exists("/blank.gif")) {
                playGif("/blank.gif", true);
            } else {
                // Fallback message if no GIFs in LittleFS
                canvas.fillScreen(TFT_BLACK);
                canvas.setTextColor(TFT_WHITE, TFT_BLACK);
                canvas.setTextSize(1);
                canvas.drawCenterString("DASAI MOCHI", 120, 100);
                canvas.setTextColor(0x7FFF, TFT_BLACK);
                canvas.drawCenterString("READY // UPLOAD GIFS", 120, 120);
                return;
            }
        }

        uint32_t now = millis();
        if (now - lastFrameTime >= (uint32_t)nextFrameDelayMs) {
            lastFrameTime = now;
            int frameDelay = 33;
            int result = gif.playFrame(true, &frameDelay);

            if (result == 0) {
                // End of GIF reached
                loopCount++;
                if (loopContinuous || loopCount < maxLoops) {
                    gif.reset();
                } else {
                    // One-shot animation completed -> return to idle loop
                    if (currentGifPath != returnAfterPath) {
                        playGif(returnAfterPath, true);
                    } else {
                        gif.reset();
                    }
                }
            }
            nextFrameDelayMs = (frameDelay > 0) ? frameDelay : 33;
        }
    }
};
