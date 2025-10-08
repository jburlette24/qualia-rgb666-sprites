# Qualia ESP32-S3 Round 480 Sprites Demo

Animated sprite showcase for the Adafruit Qualia ESP32-S3 driving the 480×480 round RGB666 display.

## Hardware Snapshot
- Board: Adafruit Qualia ESP32-S3 RGB666 (16-bit parallel RGB panel)
- Display timing: 480×480, 60 Hz timings baked into the sketch
- Touch: CST8xx capacitive controller over I²C

## Rendering Pipeline
- **Canvas rendering** – The sketch tries to allocate an `Arduino_Canvas` in PSRAM so sprites draw into an off-screen 16-bit buffer. When available the renderer blanks the canvas each frame, composites the aura + sprite art there, and then flushes the whole frame. If PSRAM cannot host the canvas the code automatically falls back to direct-on-panel drawing with incremental erases.
- **Double buffering** – With `USE_RGB_DOUBLE_FB=1`, the helper in `PanelDBuf` copies the finished canvas into the panel’s back buffer each frame and swaps it at VSYNC. This keeps the front buffer stable while the next frame is drawn, eliminating tearing and reducing flicker. If the firmware or hardware path does not expose a second buffer the renderer quietly reverts to single-buffer flushes.
- **Bounce buffer** – `RGB_BOUNCE_BUFFER_PX` allocates a small staging buffer in internal RAM. Large PSRAM-to-DMA transfers can otherwise stall; the bounce buffer feeds the panel in smoother bursts and improves frame pacing even when double buffering is off.

## Features
- Multiple sprites with animated halos and simple physics
- Optional RGB double buffering (ESP32 Arduino core 3.x / IDF5 APIs)
- Canvas-backed rendering with automatic fallback to direct mode
- Touch gestures: tap to bounce, drag to fling

## Build
- ESP32 Arduino core: 3.3.1+ (recommended)
- PSRAM enabled
- Libraries: GFX_Library_for_Arduino (this repo includes small guarded tweaks)

Open `qualia_round480_sprites.ino`, select the Qualia ESP32-S3 board profile, and upload.

## Configuration
See `config.h` for the main toggles:
- `USE_RGB_DOUBLE_FB`: `1` to request the IDF double-framebuffer API, `0` to flush directly
- `RGB_BOUNCE_BUFFER_PX`: bounce buffer size in pixels (`480*16` ≈ 15 KB works well)

When double buffering is disabled the renderer emits a short two-byte “tick” over USB serial each frame. That tiny pacing mechanism keeps the loop cadence steady and helps avoid visible tearing in single-buffer mode.

## File Guide
- `qualia_round480_sprites.ino`: sketch entry point; owns setup/loop and sprite allocation
- `config.h`: compile-time configuration knobs
- `hardware.{h,cpp}`: display, canvas, backlight, and touch bring-up
- `renderer.{h,cpp}`: frame composition and present strategy
- `PanelDBuf.{h,cpp}`: lightweight wrapper over ESP-IDF RGB double-buffer calls
- `sprites.{h,cpp}` plus `sprite_assets/*.h|*.cpp`: pixel data and blitting helpers
- `actors.h`: sprite behavior classes (bounce + fling)

## Tips
- Double buffering plus a 16-line bounce buffer delivers the smoothest animation.
- If PSRAM fragmentation prevents canvas allocation, the direct-render fallback still animates reliably thanks to incremental clears and the serial tick pacing.
