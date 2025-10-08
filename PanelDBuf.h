// Lightweight wrapper around ESP-IDF RGB panel double framebuffer APIs.
// Provides a simple present(width,height,src) to copy and swap at VSYNC.
#pragma once

#include "config.h"
#include <cstdint>

#if USE_RGB_DOUBLE_FB
#include <Arduino.h>
#include <esp_lcd_panel_rgb.h>
#include <esp_lcd_panel_ops.h>
#endif

// --- PanelDBuf --------------------------------------------------------------
class PanelDBuf
{
public:
  PanelDBuf() = default;

  // Initialize with an existing panel handle if available, otherwise noop.
  bool begin(void *panel_handle);

  // Copy a full frame to the back buffer and present it.
  // Returns true on success.
  bool present(int width, int height, const uint16_t *src);

private:
#if USE_RGB_DOUBLE_FB
  esp_lcd_panel_handle_t _panel = nullptr;
#endif
};
