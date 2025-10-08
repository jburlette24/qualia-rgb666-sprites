// Panel double-buffer helper implementation
#include "PanelDBuf.h"
#include <cstdint>

#if USE_RGB_DOUBLE_FB
#include <string.h>
#include <esp_lcd_panel_rgb.h>
#include <esp_lcd_panel_ops.h>

// --- Initialization ---------------------------------------------------------
bool PanelDBuf::begin(void *panel_handle)
{
  _panel = static_cast<esp_lcd_panel_handle_t>(panel_handle);
  return (_panel != nullptr);
}

// --- Present frame ----------------------------------------------------------
bool PanelDBuf::present(int width, int height, const uint16_t *src)
{
  if (!_panel || !src || width <= 0 || height <= 0) return false;
  void *buf = nullptr;
  if (esp_lcd_rgb_panel_get_frame_buffer(_panel, 1, &buf) != ESP_OK || buf == nullptr)
  {
    return false;
  }
  memcpy(buf, src, static_cast<size_t>(width) * static_cast<size_t>(height) * 2U);
  if (esp_lcd_panel_draw_bitmap(_panel, 0, 0, width, height, buf) != ESP_OK)
  {
    return false;
  }
  return true;
}

#else

// Stubs when the sketch is built without double-buffer support.
bool PanelDBuf::begin(void *panel_handle) { (void)panel_handle; return false; }
bool PanelDBuf::present(int, int, const uint16_t *) { return false; }

#endif
