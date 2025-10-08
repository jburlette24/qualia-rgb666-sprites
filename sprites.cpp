#include "sprites.h"
#include <Arduino_GFX_Library.h>

// --- Sprite drawing helper --------------------------------------------------
void drawSprite(Arduino_GFX &gfx, const Sprite &sprite) {
  gfx.draw16bitRGBBitmap(
    sprite.x,
    sprite.y,
    const_cast<uint16_t *>(sprite.bitmap),  // library signature wants non-const
    sprite.width,
    sprite.height
  );
}
