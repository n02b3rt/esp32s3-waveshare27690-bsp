# tools

Utility scripts for the ESP32-S3 Waveshare 27690 BSP project.

---

## png2lvgl.py

Converts a PNG image to a LVGL 9 `lv_image_dsc_t` C array (RGB565 format).

**Requirements:** Python 3 + Pillow (`pip3 install Pillow`)  
No npm, no online tools, no Node.js required.

### Usage

```bash
python3 tools/png2lvgl.py <input.png> <variable_name> [max_width] [max_height]
```

| Argument | Description | Default |
|----------|-------------|---------|
| `input.png` | Source image (any format Pillow supports) | required |
| `variable_name` | C variable name for the output | required |
| `max_width` | Max width in pixels (aspect ratio preserved) | 160 |
| `max_height` | Max height in pixels (aspect ratio preserved) | 160 |

Output files are written next to the input image.

### Example

```bash
# Convert logo, resize to max 160x160
python3 tools/png2lvgl.py ~/my_logo.png my_logo 160 160

# Output: my_logo.c  my_logo.h
# Copy to your project's main/ folder
cp my_logo.c my_logo.h examples/display_lvgl_demo/main/
```

### Using the output in LVGL 9

```c
// In CMakeLists.txt — add to SRCS:
// SRCS "main.c" "my_logo.c"

#include "my_logo.h"

lv_obj_t *img = lv_image_create(parent);
lv_image_set_src(img, &my_logo);
lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);
```

### Notes

- Output color format: `LV_COLOR_FORMAT_RGB565` (little-endian, no pre-swap)
- Byte swapping is handled by `bsp_lvgl.c` via `flags.swap_bytes = true` in `esp_lvgl_port`
- Alpha channel is discarded (converted to RGB before processing)
- Large images increase firmware binary size significantly — resize before converting
