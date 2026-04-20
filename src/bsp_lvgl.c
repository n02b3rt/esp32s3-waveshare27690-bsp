#include "bsp/bsp.h"
#include "bsp_private.h"

#include "sdkconfig.h"
#include "esp_lvgl_port.h"
#include "esp_log.h"
#include "esp_check.h"

static const char *TAG = "bsp.lvgl";

static lv_display_t *s_disp = NULL;
static lv_indev_t   *s_indev = NULL;

esp_err_t bsp_lvgl_start(lv_display_t **disp, lv_indev_t **indev)
{
    esp_lcd_panel_handle_t panel;
    esp_lcd_panel_io_handle_t io;
    esp_lcd_touch_handle_t tp;

    ESP_RETURN_ON_ERROR(bsp_display_init_internal(&panel, &io), TAG, "display");

    esp_err_t touch_ret = bsp_touch_init_internal(&tp);
    if (touch_ret != ESP_OK) {
        ESP_LOGW(TAG, "touch init failed (0x%x) — display-only mode", touch_ret);
        tp = NULL;
    }

    const lvgl_port_cfg_t lvgl_cfg = {
        .task_priority = CONFIG_BSP_LVGL_TASK_PRIORITY,
        .task_stack = CONFIG_BSP_LVGL_TASK_STACK,
        .task_affinity = CONFIG_BSP_LVGL_TASK_CORE,
        .task_max_sleep_ms = 500,
        .timer_period_ms = 5,
    };
    ESP_RETURN_ON_ERROR(lvgl_port_init(&lvgl_cfg), TAG, "lvgl_port_init");

    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = io,
        .panel_handle = panel,
        .buffer_size = BSP_LCD_H_RES * CONFIG_BSP_LVGL_BUFFER_LINES,
        .double_buffer = true,
        .hres = BSP_LCD_H_RES,
        .vres = BSP_LCD_V_RES,
        .monochrome = false,
        .rotation = {
            .swap_xy = false,
            .mirror_x = false,
            .mirror_y = false,
        },
        .flags = {
            .buff_dma = false,     /* PSRAM+DMA unreliable on ESP32-S3 */
            .buff_spiram = true,   /* LVGL buffers in PSRAM */
            .swap_bytes = true,    /* ST7789 SPI: big-endian expected, DMA sends LE */
        },
    };
    s_disp = lvgl_port_add_disp(&disp_cfg);
    ESP_RETURN_ON_FALSE(s_disp, ESP_FAIL, TAG, "lvgl_port_add_disp");

    if (tp) {
        const lvgl_port_touch_cfg_t touch_cfg = {
            .disp = s_disp,
            .handle = tp,
        };
        s_indev = lvgl_port_add_touch(&touch_cfg);
        ESP_RETURN_ON_FALSE(s_indev, ESP_FAIL, TAG, "lvgl_port_add_touch");
    }

    if (disp) *disp = s_disp;
    if (indev) *indev = s_indev;

    ESP_LOGI(TAG, "LVGL ready (%s)", tp ? "display + touch" : "display only");
    return ESP_OK;
}

bool bsp_lvgl_lock(int timeout_ms)
{
    return lvgl_port_lock(timeout_ms);
}

void bsp_lvgl_unlock(void)
{
    lvgl_port_unlock();
}
