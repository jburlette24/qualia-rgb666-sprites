#pragma once

// Hardware/setup helpers for the Qualia round 480 demo
// - Initializes panel/display/canvas/backlight
// - Initializes touch controller

#include <Arduino_GFX_Library.h>
#include "canvas/Arduino_Canvas.h"
#include <Wire.h>
#include "Adafruit_CST8XX.h"
#include "config.h"

// --- Display bring-up --------------------------------------------------------
// Initialize I2C, display, optional canvas, and backlight. If the canvas
// allocates successfully, drawSurface points to it; otherwise we fall back to
// the raw panel driver. Returns true on success.
bool setup_display_and_canvas(
    Arduino_ESP32RGBPanel *&rgbpanel,
    Arduino_RGB_Display *&gfx,
    Arduino_Canvas *&canvas,
    Arduino_GFX *&drawSurface,
    bool &usingCanvas,
    bool &firstDirectFrameClear,
    Arduino_XCA9554SWSPI *&expander);

// --- Touch controller -------------------------------------------------------
// Initialize the CST8xx touch controller; updates touchOK flag on success.
bool setup_touch(Adafruit_CST8XX &ctp, bool &touchOK);
