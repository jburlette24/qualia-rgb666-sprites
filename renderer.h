// Frame rendering interface for the Qualia round 480 demo
#pragma once

#include <Arduino_GFX_Library.h>
#include "canvas/Arduino_Canvas.h"
#include "actors.h"
#include "PanelDBuf.h"
#include "config.h"

// --- Frame composition ------------------------------------------------------
// Compose and present one frame. Draws sprites into the drawSurface (canvas
// when available), then presents via double-FB or canvas flush.
void renderFrame(
    unsigned long now,
    Arduino_GFX *drawSurface,
    bool &firstDirectFrameClear,
    bool usingCanvas,
    AuraSprite **sprites,
    size_t spriteCount,
    Arduino_Canvas *canvas,
    Arduino_RGB_Display *gfx,
    PanelDBuf *panelDbuf);
