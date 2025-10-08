// SPDX-FileCopyrightText: 2023 Limor Fried for Adafruit Industries
// SPDX-License-Identifier: MIT
//
// Qualia ESP32-S3 Round 480 sprites demo entry point
// - Sets up panel, canvas, touch, and sprite instances
// - Main loop steps sprites and renders via renderer module
#include "config.h"
#include <Arduino_GFX_Library.h>
#include "canvas/Arduino_Canvas.h"
#include <Wire.h>
#include "Adafruit_CST8XX.h"
#include "PanelDBuf.h"
#include "hardware.h"
#include "renderer.h"
#include "../libraries/GFX_Library_for_Arduino/src/Arduino_GFX_DoubleFB.h"
#if defined(ESP32)
#include <esp_heap_caps.h>
#endif

#include "sprites.h"
#include "alex.h"
#include "amelia.h"
#include "jim.h"
#include "fluffles.h"
#include "actors.h"

// --- Global display plumbing -------------------------------------------------

Arduino_XCA9554SWSPI *expander = new Arduino_XCA9554SWSPI(
    PCA_TFT_RESET, PCA_TFT_CS, PCA_TFT_SCK, PCA_TFT_MOSI,
    &Wire, 0x3F);

Arduino_ESP32RGBPanel *rgbpanel = new Arduino_ESP32RGBPanel(
    TFT_DE, TFT_VSYNC, TFT_HSYNC, TFT_PCLK,
    TFT_R1, TFT_R2, TFT_R3, TFT_R4, TFT_R5,
    TFT_G0, TFT_G1, TFT_G2, TFT_G3, TFT_G4, TFT_G5,
    TFT_B1, TFT_B2, TFT_B3, TFT_B4, TFT_B5,
    1 /* hync_polarity */, 46 /* hsync_front_porch */, 2 /* hsync_pulse_width */, 44 /* hsync_back_porch */,
    1 /* vsync_polarity */, 50 /* vsync_front_porch */, 16 /* vsync_pulse_width */, 16 /* vsync_back_porch */,
    0 /* pclk_active_neg */, GFX_NOT_DEFINED /* prefer_speed */, false /* useBigEndian */,
    0 /* de_idle_high */, 0 /* pclk_idle_high */, RGB_BOUNCE_BUFFER_PX /* bounce_buffer_size_px */);

Arduino_RGB_Display *gfx = new Arduino_RGB_Display(
    480 /* width */, 480 /* height */, rgbpanel, 0 /* rotation */, false /* auto_flush */,
    expander, GFX_NOT_DEFINED /* RST */, TL021WVC02_init_operations, sizeof(TL021WVC02_init_operations));

Arduino_Canvas *canvas = nullptr;
Arduino_GFX *drawSurface = nullptr;
bool usingCanvas = false;
bool firstDirectFrameClear = true;
PanelDBuf panelDbuf;

Adafruit_CST8XX ctp;
bool touchOK = false;


BouncingSprite *jimSprite = nullptr;
BouncingSprite *alexSprite = nullptr;
BouncingSprite *ameliaSprite = nullptr;
FlingableSprite *flufflesSprite = nullptr;

// --- Demo actors --------------------------------------------------------------
// Generic sprite list for stepping and rendering
AuraSprite *gSprites[8];
size_t gSpriteCount = 0;

// Build sprite objects and register them in the shared list so renderer/loop
// can treat them uniformly.
void initializeSprites()
{
  if (!drawSurface) {
    return;
  }

  bool incremental = !usingCanvas;

  jimSprite = new BouncingSprite(*drawSurface, Jim_pixels, Jim_pixels_width, Jim_pixels_height, incremental);
  alexSprite = new BouncingSprite(*drawSurface, Alex_pixels, Alex_pixels_width, Alex_pixels_height, incremental);
  ameliaSprite = new BouncingSprite(*drawSurface, Amelia_pixels, Amelia_pixels_width, Amelia_pixels_height, incremental);
  flufflesSprite = new FlingableSprite(*drawSurface, Fluffles_pixels, Fluffles_pixels_width, Fluffles_pixels_height, incremental);

  // Build the generic list (maintain order preference)
  gSpriteCount = 0;
  gSprites[gSpriteCount++] = jimSprite;
  gSprites[gSpriteCount++] = alexSprite;
  gSprites[gSpriteCount++] = ameliaSprite;
  gSprites[gSpriteCount++] = flufflesSprite;
}

// Sketch entry point: bring up hardware, initialize assets, and draw first frame.
void setup()
{
  Serial.begin(115200);
  Serial.println("Qualia Round 480 Sprites Demo");
#if defined(ESP32)
  Serial.printf("Heap free: %u, PSRAM free: %u\n",
                heap_caps_get_free_size(MALLOC_CAP_DEFAULT),
                heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
#endif
  Serial.printf("Double FB (lib): %d, (sketch): %d\n",
                (int)ARDUINO_GFX_ENABLE_DOUBLE_FB,
                (int)USE_RGB_DOUBLE_FB);
  Serial.printf("Bounce buffer px: %u\n", (unsigned)RGB_BOUNCE_BUFFER_PX);
  // No board-specific pre-init hook; keep setup minimal

  randomSeed(micros());
  if (!setup_display_and_canvas(rgbpanel, gfx, canvas, drawSurface, usingCanvas, firstDirectFrameClear, expander)) {
    while (true) { delay(10); }
  }
  Serial.printf("Render path: %s\n", usingCanvas ? "canvas" : "direct");

#if USE_RGB_DOUBLE_FB
  {
    bool ok = panelDbuf.begin(rgbpanel->getPanelHandle());
    Serial.printf("Double FB present: %s\n", ok ? "yes" : "no");
  }
#endif

  if (setup_touch(ctp, touchOK)) {
    Serial.println("Touch: ready");
  } else {
    Serial.println("Touch: not found");
  }
  initializeSprites();
  Serial.printf("Sprites initialized: %u\n", (unsigned)gSpriteCount);

  unsigned long now = millis();
  for (size_t i = 0; i < gSpriteCount; ++i) {
    if (gSprites[i]) gSprites[i]->step(now);
  }
  renderFrame(now, drawSurface, firstDirectFrameClear, usingCanvas, gSprites, gSpriteCount, canvas, gfx, &panelDbuf);
}

// Poll capacitive touch and hand events to the first sprite that claims them.
void handleTouch()
{
  if (!touchOK) {
    return;
  }

  if (!ctp.touched()) {
    for (size_t i = 0; i < gSpriteCount; ++i) {
      if (gSprites[i]) gSprites[i]->noTouch();
    }
    return;
  }

  CST_TS_Point point = ctp.getPoint(0);
  if (point.x == 0 && point.y == 0) {
    for (size_t i = 0; i < gSpriteCount; ++i) {
      if (gSprites[i]) gSprites[i]->noTouch();
    }
    return;
  }

  bool claimed = false;
  for (size_t i = 0; i < gSpriteCount; ++i) {
    if (!gSprites[i]) continue;
    if (!claimed && gSprites[i]->contains(point.x, point.y)) {
      gSprites[i]->registerTouch(point.x, point.y);
      claimed = true;
    } else {
      gSprites[i]->noTouch();
    }
  }
}

// renderFrame implementation moved to renderer.cpp

// Main animation loop: touch -> physics -> render -> small delay for ~60 FPS.
void loop()
{
  unsigned long now = millis();

  handleTouch();

  for (size_t i = 0; i < gSpriteCount; ++i) {
    if (gSprites[i]) gSprites[i]->step(now);
  }
  renderFrame(now, drawSurface, firstDirectFrameClear, usingCanvas, gSprites, gSpriteCount, canvas, gfx, &panelDbuf);
  delay(15);
}
