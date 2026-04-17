# esp32s3-waveshare27690-bsp

Board Support Package dla **Waveshare ESP32-S3-Touch-LCD-2.8** (SKU 27690).
ESP-IDF managed component z driverami display + touch + glue do LVGL.

**Hardware:**
- SoC: ESP32-S3R8 (8 MB PSRAM, 16 MB flash)
- Display: ST7789T3, 240×320, SPI
- Touch: CST328, capacitive 5-point, I²C
- Wi-Fi + BLE 5

## Co jest w środku

| Plik | Odpowiedzialność |
|------|------------------|
| `src/bsp.c` | SPI bus, I²C bus, backlight GPIO |
| `src/bsp_display.c` | init panelu ST7789 (via `espressif/esp_lcd_st7789`) |
| `src/bsp_touch.c` | init CST328 (via `waveshare/esp_lcd_touch_cst328`) |
| `src/bsp_lvgl.c` | `esp_lvgl_port` - display + touch jako LVGL devices |
| `Kconfig` | pinout do ustawienia przez `idf.py menuconfig` |

Kod Waveshare'a NIE jest vendorowany - używamy tylko ich **managed component** z ESP Component Registry (driver CST328). Display leci przez oficjalny Espressif component.

## Użycie

W `main/idf_component.yml` aplikacji:

```yaml
dependencies:
  knimakers/esp32s3_waveshare27690_bsp:
    git: https://github.com/knimakers/esp32s3-waveshare27690-bsp.git
    version: "^0.1"
```

W kodzie:

```c
#include "bsp/bsp.h"

void app_main(void) {
    ESP_ERROR_CHECK(bsp_init());
    ESP_ERROR_CHECK(bsp_display_start());

    lv_display_t *disp;
    lv_indev_t *indev;
    ESP_ERROR_CHECK(bsp_lvgl_start(&disp, &indev));

    if (bsp_lvgl_lock(1000)) {
        lv_obj_t *label = lv_label_create(lv_screen_active());
        lv_label_set_text(label, "dupa");
        lv_obj_center(label);
        bsp_lvgl_unlock();
    }
}
```

## Pinout (sprawdź ze schematem!)

Defaulty są w `Kconfig` i należy je zweryfikować ze schematem Waveshare'a **przed pierwszym flashem**. Niepasujące piny = zepsuta płytka albo brak obrazu.

Miejsca do weryfikacji:
- `BSP_LCD_PIN_MOSI`, `_SCLK`, `_CS`, `_DC`, `_RST`, `_BL`
- `BSP_TOUCH_PIN_SDA`, `_SCL`, `_RST`, `_INT`

Schemat: https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-2.8 → Resources → Schematic.

## Build przykładu

```bash
cd examples/display_lvgl_demo
idf.py set-target esp32s3
idf.py menuconfig   # zweryfikuj piny pod "Waveshare 27690 BSP"
idf.py build flash monitor
```

Oczekiwane: na ekranie "KNI Makers" + przycisk "tap me" + licznik tapnięć.

## Wersjonowanie

SemVer. `0.x.y` = pre-1.0, API może się jeszcze zmienić. Breaking changes bumpują minor.

## Licencja

Apache-2.0.
