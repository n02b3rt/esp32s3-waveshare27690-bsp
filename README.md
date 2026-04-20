# ESP32-S3 Waveshare 27690 BSP

> Board Support Package for the **Waveshare ESP32-S3-Touch-LCD-2.8** (SKU 27690)  
> ESP-IDF managed component — ST7789 display + CST328 touch + LVGL integration

[![ESP-IDF](https://img.shields.io/badge/ESP--IDF-v5.3+-blue)](https://github.com/espressif/esp-idf)
[![LVGL](https://img.shields.io/badge/LVGL-9.2-green)](https://lvgl.io)
[![License](https://img.shields.io/badge/license-Apache%202.0-blue)](LICENSE)

*Koło Naukowe Informatyków — Uniwersytet Rzeszowski*

---

## Hardware

| | |
|---|---|
| **SoC** | ESP32-S3R8 — dual-core Xtensa LX7 @ 240 MHz |
| **PSRAM** | 8 MB Octal @ 80 MHz |
| **Flash** | 16 MB QIO @ 80 MHz |
| **Display** | ST7789T3 — 2.8" IPS TFT, 240×320, SPI @ 80 MHz |
| **Touch** | CST328 — capacitive 5-point, I²C @ 400 kHz |
| **Wireless** | Wi-Fi 4 + BLE 5 |

---

## Repository layout

```
esp32s3-waveshare27690-bsp/
├── src/                        # BSP source — hardware drivers
│   ├── bsp.c                   # SPI bus, I²C bus, backlight GPIO
│   ├── bsp_display.c           # ST7789 panel init via esp_lcd
│   ├── bsp_touch.c             # CST328 touch init
│   └── bsp_lvgl.c              # esp_lvgl_port glue (display + touch → LVGL)
├── include/bsp/bsp.h           # Public API
├── priv_include/bsp_private.h  # Internal API
├── Kconfig                     # GPIO pin configuration (idf.py menuconfig)
├── idf_component.yml           # Managed component manifest
├── examples/
│   └── display_lvgl_demo/      # Full UI demo — dark theme, splash, tabs
└── tools/
    └── png2lvgl.py             # PNG → LVGL 9 C array converter
```

---

## Add to your project

**1.** Add to your project's `main/idf_component.yml`:

```yaml
dependencies:
  idf: ">=5.3"
  norek/esp32s3-waveshare27690-bsp:
    git: https://github.com/YOUR_USERNAME/esp32s3-waveshare27690-bsp.git
  lvgl/lvgl: "~9.2"
  espressif/esp_lvgl_port: "^2.7"
  waveshare/esp_lcd_touch_cst328: "^1.0"
```

**2.** Minimal `app_main`:

```c
#include "bsp/bsp.h"
#include "lvgl.h"

void app_main(void)
{
    ESP_ERROR_CHECK(bsp_init());
    ESP_ERROR_CHECK(bsp_display_start());

    lv_display_t *disp  = NULL;
    lv_indev_t   *indev = NULL;
    ESP_ERROR_CHECK(bsp_lvgl_start(&disp, &indev));

    if (bsp_lvgl_lock(1000)) {
        // build your LVGL UI here
        bsp_lvgl_unlock();
    }
}
```

**No manual pin configuration needed** — all GPIO defaults in `Kconfig` match the Waveshare 27690 board out of the box. Override via `idf.py menuconfig → BSP Configuration` if needed.

---

## API

```c
/* Board init — call once before anything else */
esp_err_t bsp_init(void);

/* Start ST7789 display panel + backlight */
esp_err_t bsp_display_start(void);
esp_err_t bsp_display_brightness_set(uint8_t brightness_pct); /* 0–100, currently on/off */

/* Start LVGL task, returns display and touch input device handles */
esp_err_t bsp_lvgl_start(lv_display_t **disp, lv_indev_t **indev);

/* Thread-safe LVGL access from application tasks */
bool      bsp_lvgl_lock(int timeout_ms);
void      bsp_lvgl_unlock(void);

/* Display resolution */
#define BSP_LCD_H_RES  240
#define BSP_LCD_V_RES  320
```

---

## Default pin mapping

| Signal | GPIO | Configurable |
|--------|------|-------------|
| SPI MOSI | 45 | `BSP_LCD_PIN_MOSI` |
| SPI SCLK | 40 | `BSP_LCD_PIN_SCLK` |
| LCD CS | 42 | `BSP_LCD_PIN_CS` |
| LCD DC | 41 | `BSP_LCD_PIN_DC` |
| LCD RST | 39 | `BSP_LCD_PIN_RST` |
| Backlight | 5 | `BSP_LCD_PIN_BL` |
| Touch SDA | 1 | `BSP_TOUCH_PIN_SDA` |
| Touch SCL | 3 | `BSP_TOUCH_PIN_SCL` |
| Touch RST | 2 | `BSP_TOUCH_PIN_RST` |
| Touch INT | 4 | `BSP_TOUCH_PIN_INT` |

Verify against the [Waveshare schematic](https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-2.8) before first flash.

---

## Demo application

[`examples/display_lvgl_demo/`](examples/display_lvgl_demo/README.md) — dark-themed UI template you can use as a starting point for new projects:

- Splash screen with animated loading bar and KNI logo
- 3-tab main screen: **HOME** (live uptime / free heap), **INFO**, **SYS** hardware table
- Vertical scroll on all content panels
- Live system stats refreshed every second

Use it as a copy-paste template when starting a new project on this board.

---

## Tools

[`tools/`](tools/README.md) — `png2lvgl.py`: converts any PNG to a LVGL 9 `lv_image_dsc_t` C array using Python + Pillow. No npm, no online tools required.

---

## Dependencies

| Component | Version | Role |
|-----------|---------|------|
| `espressif/esp_lvgl_port` | ^2.7 | LVGL ↔ esp_lcd bridge |
| `lvgl/lvgl` | ~9.2 | Graphics library |
| `waveshare/esp_lcd_touch_cst328` | ^1.0 | CST328 touch driver |
| ESP-IDF `esp_lcd_st7789` | built-in | ST7789 display driver |

---

## Versioning

SemVer. `0.x.y` = pre-1.0, minor API changes possible. Breaking changes bump minor.

## License

Apache 2.0 — see [LICENSE](LICENSE)
