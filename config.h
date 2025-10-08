// Compile-time options for the Qualia round 480 demo
#pragma once

// Enable RGB double buffering (requires ESP32 Arduino core 3.x / IDF5 APIs)
// 0 = disabled (default), 1 = enabled
#ifndef USE_RGB_DOUBLE_FB
#define USE_RGB_DOUBLE_FB 1
#endif

// Bounce buffer size in pixels for the RGB panel DMA path.
// Helps smooth PSRAM bursts by staging transfers in internal RAM.
// Recommended: width * 16 (e.g., 480*16 = 7680 px -> ~15 KB)
#ifndef RGB_BOUNCE_BUFFER_PX
#define RGB_BOUNCE_BUFFER_PX (480 * 16)
#endif
