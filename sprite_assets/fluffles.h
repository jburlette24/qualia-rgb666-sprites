#pragma once

#include <Arduino.h>

inline constexpr uint16_t Fluffles_pixels_width  = 86;
inline constexpr uint16_t Fluffles_pixels_height = 89;

// Pixel data is defined in fluffles.cpp
extern const uint16_t Fluffles_pixels[] PROGMEM;
