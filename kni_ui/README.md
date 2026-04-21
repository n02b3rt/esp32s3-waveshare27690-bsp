# kni_ui

Dark-themed UI component for the KNI ESP32-S3 platform.  
Drop it into any project via `idf_component.yml` and get a ready-made splash screen + tabbed main screen.

---

## What you get

- **Splash screen** — animated teal loading bar, optional logo image, fade-in transition
- **Tabbed main screen** — top status bar, 1–5 tabs, scrollable content panels, bottom menu bar
- **Content helpers** — `kni_ui_card`, `kni_ui_kv_row`, `kni_ui_text`, `kni_ui_col`
- **Color palette** — dark navy theme, overridable via `#define` before `#include`

---

## Quick start

### 1. Add to `idf_component.yml`

```yaml
dependencies:
  kni_ui:
    git: https://github.com/kni-ur/esp32s3-waveshare27690-bsp.git
    path: kni_ui
```

Or if this repo is your BSP (already in `components/`), just add to CMakeLists REQUIRES:

```cmake
REQUIRES esp32s3-waveshare27690-bsp kni_ui
```

### 2. Write tab builders

```c
#include "kni_ui/kni_ui.h"

static lv_obj_t *s_uptime = NULL;

static void build_home(lv_obj_t *panel)
{
    lv_obj_t *col  = kni_ui_col(panel);           // flex wrapper for multiple cards
    lv_obj_t *card = kni_ui_card(col, "STATUS");
    s_uptime = kni_ui_kv_row(card, "Uptime", "00:00:00");
    // update later: lv_label_set_text_fmt(s_uptime, "%02d:%02d:%02d", h, m, s);
}

static void build_info(lv_obj_t *panel)
{
    lv_obj_t *card = kni_ui_card(panel, "ABOUT");
    kni_ui_text(card, "Your project description here.", KNI_UI_C_TEXT);
}
```

### 3. Configure and start

```c
static const kni_ui_tab_t tabs[] = {
    { "HOME", build_home },
    { "INFO", build_info },
};

static const kni_ui_cfg_t ui_cfg = {
    .title     = "MY PROJECT",
    .subtitle  = "ESP32-S3",
    .logo      = &my_logo,          // lv_image_dsc_t*, or NULL to skip
    .splash_ms = 2800,              // loading bar duration in ms
    .tabs      = tabs,
    .tab_count = 2,
};

// inside bsp_lvgl_lock():
ESP_ERROR_CHECK(kni_ui_start(&ui_cfg));
```

---

## API reference

### `kni_ui_start(cfg)`

Validates config, builds the splash screen, starts the animated loading bar.  
Call from within `bsp_lvgl_lock()` / `bsp_lvgl_unlock()`.  
Returns `ESP_ERR_INVALID_ARG` if tab count is 0 or > `KNI_UI_MAX_TABS` (5).

### `kni_ui_card(parent, title)`

Creates a styled card with a teal title label.  
Returns the card object — pass it to `kni_ui_kv_row` / `kni_ui_text`.

```c
lv_obj_t *card = kni_ui_card(panel, "NETWORK");
```

### `kni_ui_kv_row(card, key, value)`

Adds a horizontal key → value row to a card.  
Returns the **value label** — keep the pointer to update it at runtime.

```c
lv_obj_t *ip = kni_ui_kv_row(card, "IP", "0.0.0.0");
// later:
lv_label_set_text(ip, "192.168.1.42");
```

### `kni_ui_text(card, text, color)`

Adds a wrapped paragraph inside a card.  
Pass `KNI_UI_C_TEXT` or `KNI_UI_C_SUBTEXT` for the color.

```c
kni_ui_text(card, "Long description that wraps automatically.", KNI_UI_C_SUBTEXT);
```

### `kni_ui_col(panel)`

Creates a transparent flex-column wrapper inside a panel.  
Use this when a tab needs **more than one card** stacked vertically.

```c
lv_obj_t *col = kni_ui_col(panel);
lv_obj_t *c1  = kni_ui_card(col, "CARD ONE");
lv_obj_t *c2  = kni_ui_card(col, "CARD TWO");
```

---

## Color palette

All colors are `#define`s — redefine them before `#include "kni_ui/kni_ui.h"` to override.

| Macro | Default | Role |
|---|---|---|
| `KNI_UI_C_BG` | `0x07111A` | Screen background |
| `KNI_UI_C_CARD` | `0x16263A` | Card surface |
| `KNI_UI_C_TEAL` | `0x00CCA3` | Accent / active |
| `KNI_UI_C_TEAL_DIM` | `0x00856B` | Dimmed accent |
| `KNI_UI_C_TEXT` | `0xDDECF8` | Primary text |
| `KNI_UI_C_SUBTEXT` | `0x6888A8` | Secondary text |
| `KNI_UI_C_BORDER` | `0x1D3148` | Card border |
| `KNI_UI_C_BAR` | `0x0B1825` | Top / bottom bars |
| `KNI_UI_C_INACTIVE` | `0x3D5A78` | Inactive tab label |

---

## Logo image

Convert any PNG with the bundled script:

```bash
python3 tools/png2lvgl.py logo.png kni_logo 160 160
```

This outputs `kni_logo.c` + `kni_logo.h`.  
Include the `.c` in your `CMakeLists.txt` SRCS and pass `&kni_logo` to `ui_cfg.logo`.

See [`tools/README.md`](../tools/README.md) for full usage.

---

## Dependencies

| Component | Version |
|---|---|
| ESP-IDF | >= 5.3 |
| lvgl/lvgl | ~9.2 |
| espressif/esp_lvgl_port | ^2.7 |
| esp32s3-waveshare27690-bsp | this repo |
