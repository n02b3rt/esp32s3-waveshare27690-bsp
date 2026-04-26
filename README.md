# ESP32-S3 Waveshare 27690 BSP

> Board Support Package for the **Waveshare ESP32-S3-Touch-LCD-2.8** (SKU 27690)  
> ST7789 display · CST328 capacitive touch · LVGL 9 integration

[![ESP-IDF](https://img.shields.io/badge/ESP--IDF-v5.3+-blue?logo=espressif&logoColor=white)](https://github.com/espressif/esp-idf)
[![LVGL](https://img.shields.io/badge/LVGL-9.2-brightgreen?logo=lvgl&logoColor=white)](https://lvgl.io)
[![Target](https://img.shields.io/badge/target-ESP32--S3-orange?logo=espressif&logoColor=white)](https://www.espressif.com/en/products/socs/esp32-s3)
[![License](https://img.shields.io/badge/license-Apache%202.0-blue)](LICENSE)
[![Version](https://img.shields.io/badge/version-0.1.0-informational)](idf_component.yml)

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
├── src/                    # BSP source — SPI/I²C bus, display, touch, LVGL glue
├── include/bsp/bsp.h       # Public API
├── Kconfig                 # GPIO pin config (idf.py menuconfig → BSP Configuration)
├── idf_component.yml       # Managed component manifest
├── kni_ui/                 # ← optional dark-theme UI component
├── examples/
│   └── display_lvgl_demo/  # ← full UI demo — splash, tabs, live stats
└── tools/                  # ← PNG → LVGL converter script
```

Each subdirectory has its own README:

| Directory | Description |
|---|---|
| [`kni_ui/`](kni_ui/README.md) | Ready-made LVGL UI component — splash screen, tabs, card helpers |
| [`examples/display_lvgl_demo/`](examples/display_lvgl_demo/README.md) | Full demo app — build, flash, and use as a project template |
| [`tools/`](tools/README.md) | `png2lvgl.py` — PNG to LVGL 9 C array converter |

---

## Add to your project

**1.** In your `main/idf_component.yml`:

```yaml
dependencies:
  idf: ">=5.3"
  norek/esp32s3-waveshare27690-bsp:
    git: https://github.com/n02b3rt/esp32s3-waveshare27690-bsp
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

All GPIO defaults in `Kconfig` match the Waveshare 27690 board out of the box — no manual pin config needed. Override via `idf.py menuconfig → BSP Configuration` if needed.

---

## API

```c
esp_err_t bsp_init(void);                                        // call once, before anything else
esp_err_t bsp_display_start(void);                               // start ST7789 + backlight
esp_err_t bsp_display_brightness_set(uint8_t brightness_pct);    // 0–100 (currently on/off)
esp_err_t bsp_lvgl_start(lv_display_t **disp, lv_indev_t **indev); // start LVGL task
bool      bsp_lvgl_lock(int timeout_ms);                         // grab LVGL mutex
void      bsp_lvgl_unlock(void);

#define BSP_LCD_H_RES  240
#define BSP_LCD_V_RES  320
```

---

## Pin mapping

| Signal | GPIO | Kconfig symbol |
|--------|:----:|----------------|
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

[![Demo](https://img.shields.io/badge/example-display__lvgl__demo-blueviolet)](examples/display_lvgl_demo/README.md)

[`examples/display_lvgl_demo/`](examples/display_lvgl_demo/README.md) — dark-themed UI template built on top of `kni_ui`:

- KNI logo splash with animated loading bar
- 3-tab layout: **HOME** (live uptime / free heap), **INFO**, **SYS** hardware table
- Scrollable content panels, live stats refreshed every second

Use it as a copy-paste base when starting a new project on this board.  
→ See [`examples/display_lvgl_demo/README.md`](examples/display_lvgl_demo/README.md) for build instructions and templating guide.

---

## kni_ui component

[![kni_ui](https://img.shields.io/badge/component-kni__ui-teal)](kni_ui/README.md)

[`kni_ui/`](kni_ui/README.md) — optional UI component that wraps LVGL into a ready-made shell:
splash screen, top status bar, tabbed layout, scrollable panels, and card-based content helpers.

→ See [`kni_ui/README.md`](kni_ui/README.md) for the full API and color palette reference.

---

## Tools

[![Tools](https://img.shields.io/badge/tool-png2lvgl.py-yellow)](tools/README.md)

[`tools/png2lvgl.py`](tools/README.md) — converts any PNG to a LVGL 9 `lv_image_dsc_t` C array (RGB565).  
Python 3 + Pillow only. No npm, no online tools.

```bash
python3 tools/png2lvgl.py logo.png my_logo 160 160
# → my_logo.c  my_logo.h
```

→ See [`tools/README.md`](tools/README.md) for full usage.

---

## License

Apache 2.0 — see [LICENSE](LICENSE)
