#pragma once
#include <cstdint>

class Arduino_GFX;

// --- Sprite primitive -------------------------------------------------------
struct Sprite {
    int16_t x;      // current x position
    int16_t y;      // current y position
    uint16_t width;
    uint16_t height;
    const uint16_t *bitmap; // staring element of the bitmap array


    // int16_t vx;     // velocity x
    // int16_t vy;     // velocity y
};

void drawSprite(Arduino_GFX &gfx, const Sprite &sprite);
