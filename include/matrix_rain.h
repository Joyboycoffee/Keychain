#pragma once
#include <LovyanGFX.hpp>
#include "display_setup.h"
#include "config.h"

#define MATRIX_COLS 16

class MatrixRain {
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

    void update() {
        uint32_t now = millis();
        if (now - lastUpdate < 30) return; // 33 FPS tick
        lastUpdate = now;

        // Semi-transparent fade effect
        canvas.fillScreen(TFT_BLACK);

        for (int i = 0; i < MATRIX_COLS; i++) {
            int x = i * 15 + 4;
            
            // Draw green trail
            for (int t = 1; t <= 8; t++) {
                int trailY = yPos[i] - (t * 14);
                if (trailY >= 0 && trailY < 240) {
                    uint8_t greenDim = 255 - (t * 28);
                    uint16_t col = canvas.color565(0, greenDim, 0);
                    canvas.setTextColor(col, TFT_BLACK);
                    canvas.setTextSize(1);
                    canvas.drawChar(getRandomChar(), x, trailY);
                }
            }

            // Draw bright glowing head character
            if (yPos[i] >= 0 && yPos[i] < 240) {
                canvas.setTextColor(TFT_WHITE, TFT_BLACK);
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

        // Overlay status header
        canvas.setTextColor(0x07E0, TFT_BLACK);
        canvas.setTextSize(1);
        canvas.drawString("NEO_MATRIX // v1.0", 10, 10);
    }

private:
    char getRandomChar() {
        int r = random(0, 36);
        if (r < 10) return '0' + r;
        return 'A' + (r - 10);
    }
};
