// Hardware/setup helpers implementation
// See hardware.h for responsibilities.
#include "hardware.h"

bool setup_display_and_canvas(
    Arduino_ESP32RGBPanel *&rgbpanel,
    Arduino_RGB_Display *&gfx,
    Arduino_Canvas *&canvas,
    Arduino_GFX *&drawSurface,
    bool &usingCanvas,
    bool &firstDirectFrameClear,
    Arduino_XCA9554SWSPI *&expander)
{
  // --- I2C and GPIO expander -------------------------------------------------
  // I2C for the PCA95xx expander and touch controller
  Wire.begin();
  Wire.setClock(400000);

  // Expander for backlight
  expander = new Arduino_XCA9554SWSPI(
      PCA_TFT_RESET, PCA_TFT_CS, PCA_TFT_SCK, PCA_TFT_MOSI,
      &Wire, 0x3F);

  // --- Canvas allocation -----------------------------------------------------
  // Panel with bounce buffer configured in sketch via constructor args.
  // rgbpanel is constructed in the sketch to keep pin/timing nearby.

  // Try to allocate a canvas first (before panel begins) to improve
  // chances of large contiguous allocation in PSRAM. Use
  // GFX_SKIP_OUTPUT_BEGIN so we don't touch the output yet.
  canvas = new Arduino_Canvas(gfx->width(), gfx->height(), gfx);
  if (canvas && canvas->begin(GFX_SKIP_OUTPUT_BEGIN)) {
    drawSurface = canvas;
    usingCanvas = true;
    firstDirectFrameClear = false;
  } else {
    if (canvas) { delete canvas; canvas = nullptr; }
    drawSurface = gfx;
    usingCanvas = false;
    firstDirectFrameClear = true;
  }

  // --- Panel begin & first frame --------------------------------------------
  // After the drawing surface is settled, bring up the panel.
  if (!gfx->begin()) {
    return false;
  }

  // Prime the screen to black before enabling the backlight.
  if (usingCanvas) {
    canvas->fillScreen(BLACK);
    canvas->flush();
    gfx->flush(true);
  } else {
    gfx->fillScreen(BLACK);
    gfx->flush(true);
  }

  // --- Backlight -------------------------------------------------------------
  // Panel is ready; turn the backlight on via the expander.
  expander->pinMode(PCA_TFT_BACKLIGHT, OUTPUT);
  expander->digitalWrite(PCA_TFT_BACKLIGHT, HIGH);

  return true;
}

bool setup_touch(Adafruit_CST8XX &ctp, bool &touchOK)
{
  const uint8_t I2C_TOUCH_ADDR = 0x15;
  if (!ctp.begin(&Wire, I2C_TOUCH_ADDR)) {
    touchOK = false;
    return false;
  }
  touchOK = true;
  return true;
}
