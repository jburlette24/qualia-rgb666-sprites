// Frame rendering implementation
#include "renderer.h"

// --- Optional pacing helper -------------------------------------------------
// Minimal per-frame serial activity to provide smoothing when not using
// double buffering. Two NUL bytes are written if space is available.
static inline void frame_tick()
{
#if !USE_RGB_DOUBLE_FB
  if (Serial && Serial.availableForWrite() >= 2)
  {
    uint8_t z[2] = {0, 0};
    Serial.write(z, 2);
  }
#endif
}

void renderFrame(
    unsigned long now,
    Arduino_GFX *drawSurface,
    bool &firstDirectFrameClear,
    bool usingCanvas,
    AuraSprite **sprites,
    size_t spriteCount,
    Arduino_Canvas *canvas,
    Arduino_RGB_Display *gfx,
    PanelDBuf *panelDbuf)
{
  // --- Validate draw surface -------------------------------------------------
  if (!drawSurface)
  {
    return;
  }

  // --- Prepare background ----------------------------------------------------
  if (usingCanvas)
  {
    drawSurface->fillScreen(BLACK);
    frame_tick();
  }
  else if (firstDirectFrameClear)
  {
    drawSurface->fillScreen(BLACK);
    firstDirectFrameClear = false;
  }

  // --- Sprite rendering ------------------------------------------------------
  for (size_t i = 0; i < spriteCount; ++i)
  {
    if (sprites[i])
    {
      sprites[i]->render(now, true);
    }
  }

  // --- Present ----------------------------------------------------------------
  if (usingCanvas)
  {
#if USE_RGB_DOUBLE_FB
    uint16_t *src = canvas ? canvas->getFramebuffer() : nullptr;
    if (!(panelDbuf && src && panelDbuf->present(canvas->width(), canvas->height(), src)))
    {
      if (canvas)
      {
        canvas->flush();
        gfx->flush(true);
      }
    }
#else
    if (canvas)
    {
      canvas->flush();
      gfx->flush(true);
    }
#endif
  }
  else
  {
    if (gfx)
    {
      gfx->flush(true);
    }
  }
}
