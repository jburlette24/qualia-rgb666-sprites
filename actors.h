// Sprite classes and helpers used in the demo.
// AuraSprite provides common rendering and optional touch hooks;
// BouncingSprite and FlingableSprite implement simple behaviors.
#pragma once

#include <Arduino_GFX_Library.h>
#include "sprites.h"

// --- Utility helpers --------------------------------------------------------
static constexpr uint16_t AURA_COLORS[3] = {MAGENTA, CYAN, YELLOW};

static inline int16_t clampCoord(int16_t value, int16_t minValue, int16_t maxValue)
{
  if (value < minValue) return minValue;
  if (value > maxValue) return maxValue;
  return value;
}

// --- AuraSprite base class ---------------------------------------------------
class AuraSprite
{
public:
  AuraSprite(Arduino_GFX &display,
             const uint16_t *bitmap,
             uint16_t width,
             uint16_t height,
             bool incremental)
      : gfx(display), incrementalClear(incremental)
  {
    int16_t startX = (gfx.width() - static_cast<int16_t>(width)) / 2;
    int16_t startY = (gfx.height() - static_cast<int16_t>(height)) / 2;
    sprite = {startX, startY, width, height, bitmap};
  }

  const Sprite &getSprite() const { return sprite; }

  void render(unsigned long nowMs, bool animateAura)
  {
    if (incrementalClear) {
      gfx.startWrite();
      if (!firstFrame) {
        clearPrevious();
      }
      drawAura(nowMs, animateAura);
      drawSprite(gfx, sprite);
      gfx.endWrite();
      lastX = sprite.x;
      lastY = sprite.y;
      firstFrame = false;
    } else {
      drawAura(nowMs, animateAura);
      drawSprite(gfx, sprite);
      lastAuraRadii[0] = lastAuraRadii[1] = lastAuraRadii[2] = 0;
      firstFrame = false;
    }
  }

  // Optional per-frame physics/update; default no-op so arrays of AuraSprite*
  // can be stepped generically.
  virtual void step(unsigned long) {}

  // Touch interaction hooks; default to no-op/non-hit so arrays of
  // AuraSprite* can be iterated generically for touch handling.
  virtual bool contains(int16_t, int16_t) const { return false; }
  virtual void registerTouch(int16_t, int16_t) {}
  virtual void noTouch() {}

protected:
  void clearPrevious()
  {
    gfx.fillRect(lastX, lastY, sprite.width, sprite.height, BLACK);

    int16_t centerX = lastX + (sprite.width / 2);
    int16_t centerY = lastY + (sprite.height / 2);
    for (uint8_t i = 0; i < 3; ++i) {
      if (lastAuraRadii[i] > 0) {
        gfx.fillCircle(centerX, centerY, lastAuraRadii[i] + 2, BLACK);
      }
    }
  }

  void drawAura(unsigned long nowMs, bool animateAura)
  {
    int16_t centerX = sprite.x + (sprite.width / 2);
    int16_t centerY = sprite.y + (sprite.height / 2);
    uint32_t phase = nowMs / 120;
    uint8_t colorShift = animateAura ? (phase % 3) : 0;
    uint8_t pulsePhase = animateAura ? (phase % 12) : 6;
    int16_t pulse = animateAura ? ((pulsePhase < 6) ? pulsePhase : 12 - pulsePhase) : 3;
    int16_t longestSide = (sprite.width > sprite.height) ? sprite.width : sprite.height;
    int16_t baseRadius = (longestSide / 2) + 10;

    for (uint8_t i = 0; i < 3; ++i) {
      int16_t radius = baseRadius + (i * 6) + pulse;
      lastAuraRadii[i] = radius;
      gfx.drawCircle(centerX, centerY, radius, AURA_COLORS[(colorShift + i) % 3]);
    }
  }

  Arduino_GFX &gfx;
  Sprite sprite{};
  int16_t lastX = 0;
  int16_t lastY = 0;
  int16_t lastAuraRadii[3] = {0, 0, 0};
  bool firstFrame = true;
  bool incrementalClear = true;
};

// --- BouncingSprite (edge bounce physics) -----------------------------------
class BouncingSprite : public AuraSprite
{
public:
  BouncingSprite(Arduino_GFX &display,
                 const uint16_t *bitmap,
                 uint16_t width,
                 uint16_t height,
                 bool incremental)
      : AuraSprite(display, bitmap, width, height, incremental)
  {
    xIncreasing = random(0, 2) == 1;
    yIncreasing = random(0, 2) == 1;
    xSpeed = randomSpeed();
    ySpeed = randomSpeed();
    minX = 0;
    minY = 0;
    maxX = clampCoord(display.width() - static_cast<int16_t>(width), 0, display.width());
    maxY = clampCoord(display.height() - static_cast<int16_t>(height), 0, display.height());
  }

  void step(unsigned long nowMs) override
  {
    int16_t x = sprite.x;
    int16_t y = sprite.y;

    if (x + xSpeed >= maxX || x - xSpeed <= minX) {
      xIncreasing = !xIncreasing;
      xSpeed = randomSpeed();
    }
    if (y + ySpeed >= maxY || y - ySpeed <= minY) {
      yIncreasing = !yIncreasing;
      ySpeed = randomSpeed();
    }

    if (xIncreasing) {
      x = clampCoord(x + xSpeed, minX, maxX);
    } else {
      x = clampCoord(x - xSpeed, minX, maxX);
    }

    if (yIncreasing) {
      y = clampCoord(y + ySpeed, minY, maxY);
    } else {
      y = clampCoord(y - ySpeed, minY, maxY);
    }

    sprite.x = x;
    sprite.y = y;
  }

private:
  uint8_t randomSpeed() const { return static_cast<uint8_t>(random(6, 10)); }

  bool xIncreasing = true;
  bool yIncreasing = true;
  uint8_t xSpeed = 1;
  uint8_t ySpeed = 1;
  int16_t minX = 0;
  int16_t minY = 0;
  int16_t maxX = 0;
  int16_t maxY = 0;
};

// --- FlingableSprite (touch fling behavior) ---------------------------------
class FlingableSprite : public AuraSprite
{
public:
  FlingableSprite(Arduino_GFX &display,
                  const uint16_t *bitmap,
                  uint16_t width,
                  uint16_t height,
                  bool incremental)
      : AuraSprite(display, bitmap, width, height, incremental)
  {
    halfWidth = sprite.width / 2;
    halfHeight = sprite.height / 2;
    maxX = clampCoord(display.width() - static_cast<int16_t>(sprite.width), 0, display.width());
    maxY = clampCoord(display.height() - static_cast<int16_t>(sprite.height), 0, display.height());
  }

  void registerTouch(int16_t x, int16_t y) override
  {
    prevTouchX = touchX;
    prevTouchY = touchY;
    prevTouchValid = touchValid;
    touchX = x;
    touchY = y;
    touchValid = true;
    dragInProgress = true;
  }

  void noTouch() override
  {
    dragInProgress = false;
  }

  bool contains(int16_t x, int16_t y) const override
  {
    return (x >= sprite.x && x <= sprite.x + static_cast<int16_t>(sprite.width) &&
            y >= sprite.y && y <= sprite.y + static_cast<int16_t>(sprite.height));
  }

  void step(unsigned long nowMs) override
  {
    if (dragInProgress && touchValid) {
      moveTo(touchX, touchY);
      velocityX = 0;
      velocityY = 0;
      return;
    }

    if (touchValid && prevTouchValid) {
      velocityX = touchX - prevTouchX;
      velocityY = touchY - prevTouchY;
    }

    touchValid = false;
    prevTouchValid = false;

    if (velocityX == 0 && velocityY == 0) {
      return;
    }

    int16_t newX = sprite.x + velocityX;
    int16_t newY = sprite.y + velocityY;

    if (newX < 0) {
      newX = 0;
      velocityX = -velocityX;
    } else if (newX > maxX) {
      newX = maxX;
      velocityX = -velocityX;
    }

    if (newY < 0) {
      newY = 0;
      velocityY = -velocityY;
    } else if (newY > maxY) {
      newY = maxY;
      velocityY = -velocityY;
    }

    sprite.x = newX;
    sprite.y = newY;

    velocityX = applyFriction(velocityX);
    velocityY = applyFriction(velocityY);
  }

private:
  void moveTo(int16_t x, int16_t y)
  {
    int16_t centeredX = x - halfWidth;
    int16_t centeredY = y - halfHeight;
    sprite.x = clampCoord(centeredX, 0, maxX);
    sprite.y = clampCoord(centeredY, 0, maxY);
  }

  static int16_t applyFriction(int16_t value)
  {
    if (value > 0) {
      value -= 1;
      if (value < 0) {
        value = 0;
      }
    } else if (value < 0) {
      value += 1;
      if (value > 0) {
        value = 0;
      }
    }
    return value;
  }

  bool dragInProgress = false;
  bool touchValid = false;
  bool prevTouchValid = false;
  int16_t touchX = 0;
  int16_t touchY = 0;
  int16_t prevTouchX = 0;
  int16_t prevTouchY = 0;
  int16_t velocityX = 0;
  int16_t velocityY = 0;
  int16_t halfWidth = 0;
  int16_t halfHeight = 0;
  int16_t maxX = 0;
  int16_t maxY = 0;
};
