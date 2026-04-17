/*
 * Minimal smoke test for BSP:
 *   1) boot board, init SPI/I2C, backlight
 *   2) init display + touch
 *   3) boot LVGL
 *   4) show "KNI Makers" label + a button that counts taps
 *
 * If you see a label and tap count increments on touch -> BSP OK.
 */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "bsp/bsp.h"
#include "lvgl.h"

static const char *TAG = "demo";
static int s_taps = 0;
static lv_obj_t *s_tap_label;

static void on_btn_click(lv_event_t *e)
{
    (void)e;
    s_taps++;
    lv_label_set_text_fmt(s_tap_label, "taps: %d", s_taps);
}

static void build_ui(void)
{
    lv_obj_t *scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x101820), 0);

    lv_obj_t *title = lv_label_create(scr);
    lv_label_set_text(title, "KNI Makers");
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFD700), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);

    lv_obj_t *subtitle = lv_label_create(scr);
    lv_label_set_text(subtitle, "Waveshare 27690 BSP");
    lv_obj_set_style_text_color(subtitle, lv_color_hex(0xAAAAAA), 0);
    lv_obj_align_to(subtitle, title, LV_ALIGN_OUT_BOTTOM_MID, 0, 8);

    lv_obj_t *btn = lv_btn_create(scr);
    lv_obj_set_size(btn, 160, 60);
    lv_obj_center(btn);
    lv_obj_add_event_cb(btn, on_btn_click, LV_EVENT_CLICKED, NULL);

    lv_obj_t *btn_lbl = lv_label_create(btn);
    lv_label_set_text(btn_lbl, "tap me");
    lv_obj_center(btn_lbl);

    s_tap_label = lv_label_create(scr);
    lv_label_set_text(s_tap_label, "taps: 0");
    lv_obj_align(s_tap_label, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_set_style_text_color(s_tap_label, lv_color_hex(0xFFFFFF), 0);
}

void app_main(void)
{
    ESP_LOGI(TAG, "BSP demo boot");
    ESP_ERROR_CHECK(bsp_init());
    ESP_ERROR_CHECK(bsp_display_start());

    lv_display_t *disp = NULL;
    lv_indev_t *indev = NULL;
    ESP_ERROR_CHECK(bsp_lvgl_start(&disp, &indev));

    if (bsp_lvgl_lock(1000)) {
        build_ui();
        bsp_lvgl_unlock();
    }

    ESP_LOGI(TAG, "running");
}
