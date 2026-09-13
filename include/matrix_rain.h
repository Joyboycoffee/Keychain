#pragma once
#include <LovyanGFX.hpp>
#include "display_setup.h"
#include "config.h"

#define MATRIX_COLS 16

class MatrixRain {
public:
    int currentTheme = 0; // 0 = Neo Green, 1 = Cyber Cyan, 2 = Synth Magenta, 3 = Fire Amber
    bool isHeartRainActive = false;
    uint32_t heartRainStartTime = 0;

private:
    int yPos[MATRIX_COLS];
    int speed[MATRIX_COLS];
    char characters[MATRIX_COLS];
    uint32_t lastUpdate = 0;

public:
    MatrixRain() {
        for (int i = 0; i < MATRIX_COLS; i++) {
            yPos[i] = random(-240, 0);
            speed[i] = random(4, 12);
            characters[i] = getRandomChar();
        }
    }

    void setTheme(int th) {
        currentTheme = (th >= 0 && th < 4) ? th : 0;
    }

    int cycleTheme() {
        currentTheme = (currentTheme + 1) % 4;
        return currentTheme;
    }

    void triggerHeartRain(uint32_t durMs = 5000) {
        isHeartRainActive = true;
        heartRainStartTime = millis();
    }

    void update() {
        uint32_t now = millis();
        if (now - lastUpdate < 30) return; // 33 FPS tick
        lastUpdate = now;

        if (isHeartRainActive && (now - heartRainStartTime >= 5000)) {
            isHeartRainActive = false;
        }

        // Black canvas
        canvas.fillScreen(TFT_BLACK);

        for (int i = 0; i < MATRIX_COLS; i++) {
            int x = i * 15 + 4;
            
            // Draw color fading trail
            for (int t = 1; t <= 8; t++) {
                int trailY = yPos[i] - (t * 14);
                if (trailY >= 0 && trailY < 240) {
                    uint8_t dim = 255 - (t * 28);
                    uint16_t col;
                    if (isHeartRainActive) {
                        col = canvas.color565(dim, 0, (uint8_t)(dim * 0.85f)); // Pink / Magenta Heart Glow
                    } else if (currentTheme == 0) {
                        col = canvas.color565(0, dim, 0); // Green
                    } else if (currentTheme == 1) {
                        col = canvas.color565(0, dim, dim); // Cyan
                    } else if (currentTheme == 2) {
                        col = canvas.color565(dim, 0, dim); // Magenta
                    } else {
                        col = canvas.color565(dim, (uint8_t)(dim * 0.65f), 0); // Amber
                    }

                    canvas.setTextColor(col, TFT_BLACK);
                    canvas.setTextSize(1);
                    canvas.drawChar(getRandomChar(), x, trailY);
                }
            }

            // Draw bright glowing head character
            if (yPos[i] >= 0 && yPos[i] < 240) {
                canvas.setTextColor(isHeartRainActive ? 0xFBE4 : TFT_WHITE, TFT_BLACK);
                canvas.setTextSize(1);
                canvas.drawChar(characters[i], x, yPos[i]);
            }

            yPos[i] += speed[i];
            if (yPos[i] > 260) {
                yPos[i] = 0;
                speed[i] = random(4, 12);
                characters[i] = getRandomChar();
            }
        }

        // Status Header
        if (isHeartRainActive) {
            canvas.setTextColor(0xF81F, TFT_BLACK);
            canvas.setTextSize(1);
            canvas.drawString("CYBER_HEART // LOVE RAIN <3", 10, 10);
        } else {
            const char* themeHeaders[] = {
                "NEO_MATRIX // GREEN",
                "CYBER_MATRIX // CYAN",
                "SYNTH_MATRIX // MAGENTA",
                "FIRE_MATRIX // AMBER"
            };
            const uint16_t themeColors[] = { 0x07E0, 0x07FF, 0xF81F, 0xFD20 };

            canvas.setTextColor(themeColors[currentTheme], TFT_BLACK);
            canvas.setTextSize(1);
            canvas.drawString(themeHeaders[currentTheme], 10, 10);
        }
    }

private:
    char getRandomChar() {
        if (isHeartRainActive) {
            const char heartChars[] = { 'L', 'O', 'V', 'E', 'U', '!', '7', '9', 'X', '3' };
            return heartChars[random(0, sizeof(heartChars))];
        }
        int r = random(0, 36);
        if (r < 10) return '0' + r;
        return 'A' + (r - 10);
    }
};
