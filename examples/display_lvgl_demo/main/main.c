/*
 * KNI ESP32-S3 Demo — kni_ui component usage template
 * Display: 240×320 ST7789, LVGL 9.2
 */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_heap_caps.h"

#include "bsp/bsp.h"
#include "kni_ui/kni_ui.h"
#include "kni_logo.h"

static const char *TAG = "kni";

/* ── Live-update label pointers (set in build_home, used by timer) ─────── */
static lv_obj_t *s_uptime_lbl = NULL;
static lv_obj_t *s_heap_lbl   = NULL;

static void sys_timer_cb(lv_timer_t *t)
{
    (void)t;
    uint64_t up  = esp_timer_get_time() / 1000000ULL;
    uint32_t h   = (uint32_t)(up / 3600);
    uint32_t m   = (uint32_t)((up % 3600) / 60);
    uint32_t sec = (uint32_t)(up % 60);
    if (s_uptime_lbl)
        lv_label_set_text_fmt(s_uptime_lbl, "%02lu:%02lu:%02lu",
                              (unsigned long)h, (unsigned long)m, (unsigned long)sec);
    if (s_heap_lbl) {
        size_t fh = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);
        lv_label_set_text_fmt(s_heap_lbl, "%lu KB", (unsigned long)(fh / 1024));
    }
}

/* ── Tab content builders ─────────────────────────────────────────────── */

static void build_home(lv_obj_t *panel)
{
    lv_obj_t *col = kni_ui_col(panel);

    lv_obj_t *c1 = kni_ui_card(col, "KNI ESP32-S3");
    kni_ui_text(c1,
        "Waveshare 27690 BSP v0.1\n"
        "ST7789 240x320 | LVGL 9.2\n"
        "CST328 capacitive touch",
        KNI_UI_C_SUBTEXT);

    lv_obj_t *c2 = kni_ui_card(col, "SYSTEM STATUS");
    s_uptime_lbl = kni_ui_kv_row(c2, "Uptime",    "00:00:00");
    s_heap_lbl   = kni_ui_kv_row(c2, "Free heap", "--- KB");
    kni_ui_kv_row(c2, "CPU", "240 MHz");

    lv_timer_create(sys_timer_cb, 1000, NULL);
    sys_timer_cb(NULL);
}

static void build_info(lv_obj_t *panel)
{
    lv_obj_t *card = kni_ui_card(panel, "O PROJEKCIE");
    kni_ui_text(card,
        "Kolo Naukowe Informatykow\n"
        "Uniwersytetu Rzeszowskiego\n\n"
        "Projekt BSP dla ESP32-S3\n"
        "z ekranem Waveshare 27690.\n\n"
        "SPI LCD ST7789  80 MHz\n"
        "I2C Touch CST328  400 kHz\n"
        "PSRAM 8 MB | Flash 16 MB\n\n"
        "Wersja BSP: v0.1.0",
        KNI_UI_C_TEXT);
}

static void build_sys(lv_obj_t *panel)
{
    lv_obj_t *card = kni_ui_card(panel, "HARDWARE");
    kni_ui_kv_row(card, "SoC",         "ESP32-S3R8");
    kni_ui_kv_row(card, "CPU",         "Xtensa LX7 x2");
    kni_ui_kv_row(card, "Takt",        "240 MHz");
    kni_ui_kv_row(card, "Flash",       "16 MB QIO");
    kni_ui_kv_row(card, "PSRAM",       "8 MB Octal");
    kni_ui_kv_row(card, "LCD",         "ST7789T3 SPI");
    kni_ui_kv_row(card, "Rozdzielcz.", "240x320");
    kni_ui_kv_row(card, "Dotyk",       "CST328 I2C");
    kni_ui_kv_row(card, "LVGL",        "9.2");
    kni_ui_kv_row(card, "IDF",         ">= 5.3");
}

/* ── kni_ui configuration ─────────────────────────────────────────────── */

static const kni_ui_tab_t tabs[] = {
    { "HOME", build_home },
    { "INFO", build_info },
    { "SYS",  build_sys  },
};

static const kni_ui_cfg_t ui_cfg = {
    .title     = "KNI",
    .subtitle  = "ESP32-S3",
    .logo      = &kni_logo,
    .splash_ms = 2800,
    .tabs      = tabs,
    .tab_count = 3,
};

/* ── Entry point ──────────────────────────────────────────────────────── */

void app_main(void)
{
    ESP_LOGI(TAG, "KNI ESP32-S3 boot");
    ESP_ERROR_CHECK(bsp_init());
    ESP_ERROR_CHECK(bsp_display_start());

    lv_display_t *disp  = NULL;
    lv_indev_t   *indev = NULL;
    ESP_ERROR_CHECK(bsp_lvgl_start(&disp, &indev));

    if (indev) {
        lv_indev_set_scroll_limit(indev, 3);
        lv_indev_set_scroll_throw(indev, 8);
    }

    if (bsp_lvgl_lock(1000)) {
        ESP_ERROR_CHECK(kni_ui_start(&ui_cfg));
        bsp_lvgl_unlock();
    }

    ESP_LOGI(TAG, "running");
}
