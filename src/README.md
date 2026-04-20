# src — BSP driver sources

This folder contains the four BSP source files. Each layer has a single responsibility and depends only on the layer below it.

```
Application (your main.c)
    │
    ▼
bsp_lvgl.c      ← LVGL task, display + touch as LVGL input devices
    │
    ├── bsp_display.c   ← ST7789 panel init via esp_lcd
    ├── bsp_touch.c     ← CST328 touch init via esp_lcd_touch
    │
    ▼
bsp.c           ← SPI bus, I²C bus, backlight GPIO (shared hardware resources)
```

---

## bsp.c

Initializes shared hardware buses and the backlight GPIO. Safe to call multiple times (idempotent — uses static `s_initialized` guard).

- `bsp_spi_bus_init()` — SPI2 host, DMA-enabled, 80 MHz
- `bsp_i2c_bus_init()` — I²C port 0, 400 kHz
- `bsp_backlight_init()` / `bsp_backlight_set()` — GPIO output, active-high

## bsp_display.c

Initializes the ST7789T3 panel over SPI using ESP-IDF's `esp_lcd` component (built-in driver, not vendored).

Key settings:
- `rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB`
- `invert_color = true` (required by this panel)
- `bits_per_pixel = 16` (RGB565)

Returns `esp_lcd_panel_handle_t` and `esp_lcd_panel_io_handle_t` via `bsp_display_init_internal()` for use by `bsp_lvgl.c`.

## bsp_touch.c

Initializes the CST328 capacitive touch controller over I²C using the `waveshare/esp_lcd_touch_cst328` managed component.

- 5-point multitouch
- INT pin configured as interrupt input
- Returns `esp_lcd_touch_handle_t` for use by `bsp_lvgl.c`

## bsp_lvgl.c

Glue layer between the hardware drivers and LVGL 9 via `espressif/esp_lvgl_port`.

Key configuration:
- LVGL task: priority / stack / core affinity from `Kconfig`
- Display buffers: double-buffered, allocated in PSRAM (`buff_spiram = true`)
- `swap_bytes = true` — fixes ST7789 SPI byte-order mismatch with LVGL's DMA output
- Provides `bsp_lvgl_lock()` / `bsp_lvgl_unlock()` for thread-safe UI updates from application tasks

---

## Adding a new peripheral

1. Create `bsp_yourdevice.c` + declare init function in `priv_include/bsp_private.h`
2. Call `bsp_spi_bus_init()` or `bsp_i2c_bus_init()` first (safe to call multiple times)
3. Add the new source file to `CMakeLists.txt` → `SRCS` list
4. Expose public API through `include/bsp/bsp.h`
