/*
 * KNI ESP32-S3 Demo — dark theme with splash screen and tabbed main UI
 * Display: 240×320 ST7789, LVGL 9.2
 */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_heap_caps.h"

#include "bsp/bsp.h"
#include "lvgl.h"
#include "kni_logo.h"

static const char *TAG = "kni";

/* ── Palette ──────────────────────────────────────────────────────────────── */
#define C_BG        0x07111A
#define C_SURFACE   0x0F1B28
#define C_CARD      0x16263A
#define C_TEAL      0x00CCA3
#define C_TEAL_DIM  0x00856B
#define C_TEXT      0xDDECF8
#define C_SUBTEXT   0x6888A8
#define C_BAR       0x0B1825
#define C_BORDER    0x1D3148
#define C_INACTIVE  0x3D5A78

/* ── Screens & widgets ────────────────────────────────────────────────────── */
static lv_obj_t *s_splash    = NULL;
static lv_obj_t *s_main      = NULL;

#define N_TABS 3
static lv_obj_t *s_panels   [N_TABS];
static lv_obj_t *s_tab_btns [N_TABS];   /* button containers  */
static lv_obj_t *s_tab_lbls [N_TABS];   /* labels inside btns */
static int       s_active_tab = 0;

static lv_obj_t *s_uptime_lbl    = NULL;
static lv_obj_t *s_heap_lbl      = NULL;
static lv_timer_t *s_sys_timer   = NULL;

/* ══════════════════════════════════════════════════════════════════════════ */
/*  HELPERS                                                                    */
/* ══════════════════════════════════════════════════════════════════════════ */

static void style_card(lv_obj_t *o)
{
    lv_obj_set_style_bg_color(o,     lv_color_hex(C_CARD),   0);
    lv_obj_set_style_bg_opa(o,       LV_OPA_COVER,           0);
    lv_obj_set_style_border_color(o, lv_color_hex(C_BORDER), 0);
    lv_obj_set_style_border_width(o, 1,                      0);
    lv_obj_set_style_radius(o,       8,                      0);
    lv_obj_set_style_pad_all(o,      10,                     0);
    lv_obj_clear_flag(o, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(o, LV_OBJ_FLAG_CLICKABLE);  /* let touch reach scrollable parent */
}

static lv_obj_t *mk_label(lv_obj_t *parent, const char *txt,
                           uint32_t col, const lv_font_t *font)
{
    lv_obj_t *l = lv_label_create(parent);
    lv_label_set_text(l, txt);
    lv_obj_set_style_text_color(l, lv_color_hex(col), 0);
    if (font) lv_obj_set_style_text_font(l, font, 0);
    return l;
}

/* ══════════════════════════════════════════════════════════════════════════ */
/*  TAB SWITCHING                                                              */
/* ══════════════════════════════════════════════════════════════════════════ */

static void switch_tab(int idx)
{
    if (idx == s_active_tab) return;

    lv_obj_set_style_border_width(s_tab_btns[s_active_tab], 0, 0);
    lv_obj_set_style_text_color(s_tab_lbls[s_active_tab],
                                lv_color_hex(C_INACTIVE), 0);
    lv_obj_add_flag(s_panels[s_active_tab], LV_OBJ_FLAG_HIDDEN);

    s_active_tab = idx;

    lv_obj_set_style_border_color(s_tab_btns[idx], lv_color_hex(C_TEAL), 0);
    lv_obj_set_style_border_side(s_tab_btns[idx],  LV_BORDER_SIDE_TOP,   0);
    lv_obj_set_style_border_width(s_tab_btns[idx], 2,                    0);
    lv_obj_set_style_text_color(s_tab_lbls[idx], lv_color_hex(C_TEAL),   0);
    lv_obj_clear_flag(s_panels[idx], LV_OBJ_FLAG_HIDDEN);
}

static void on_tab_click(lv_event_t *e)
{
    switch_tab((int)(intptr_t)lv_event_get_user_data(e));
}

/* ══════════════════════════════════════════════════════════════════════════ */
/*  LIVE STATUS TIMER                                                          */
/* ══════════════════════════════════════════════════════════════════════════ */

static void sys_timer_cb(lv_timer_t *t)
{
    (void)t;
    uint64_t up = esp_timer_get_time() / 1000000ULL;
    uint32_t h  = (uint32_t)(up / 3600);
    uint32_t m  = (uint32_t)((up % 3600) / 60);
    uint32_t sec = (uint32_t)(up % 60);
    if (s_uptime_lbl)
        lv_label_set_text_fmt(s_uptime_lbl, "Uptime     %02lu:%02lu:%02lu",
                              (unsigned long)h, (unsigned long)m, (unsigned long)sec);

    if (s_heap_lbl) {
        size_t fh = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);
        lv_label_set_text_fmt(s_heap_lbl, "Free heap  %lu KB", (unsigned long)(fh / 1024));
    }
}

/* ══════════════════════════════════════════════════════════════════════════ */
/*  PANEL 0 — HOME                                                             */
/* ══════════════════════════════════════════════════════════════════════════ */

static lv_obj_t *make_panel(lv_obj_t *scr)
{
    lv_obj_t *p = lv_obj_create(scr);
    lv_obj_set_size(p, 240, 234);
    lv_obj_set_pos(p, 0, 28);
    lv_obj_set_style_bg_color(p,    lv_color_hex(C_BG), 0);
    lv_obj_set_style_bg_opa(p,      LV_OPA_COVER,       0);
    lv_obj_set_style_border_width(p, 0, 0);
    lv_obj_set_style_radius(p,       0, 0);
    lv_obj_set_style_pad_all(p,     10, 0);
    /* no layout — flex on panel blocks scroll for initially-hidden objects */
    lv_obj_set_scroll_dir(p, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(p, LV_SCROLLBAR_MODE_ACTIVE);
    return p;
}

static lv_obj_t *make_card(lv_obj_t *parent, const char *title)
{
    lv_obj_t *c = lv_obj_create(parent);
    lv_obj_set_width(c, lv_pct(100));
    lv_obj_set_height(c, LV_SIZE_CONTENT);
    style_card(c);
    lv_obj_set_layout(c, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(c, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(c, 5, 0);
    mk_label(c, title, C_TEAL, NULL);
    return c;
}

static void build_home_panel(lv_obj_t *scr)
{
    lv_obj_t *p = make_panel(scr);
    s_panels[0] = p;

    /* Flex column wrapper — no scroll, LV_SIZE_CONTENT, stacks cards */
    lv_obj_t *col = lv_obj_create(p);
    lv_obj_set_width(col,  lv_pct(100));
    lv_obj_set_height(col, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(col,      LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(col, 0,            0);
    lv_obj_set_style_pad_all(col,      0,            0);
    lv_obj_set_style_pad_row(col,      8,            0);
    lv_obj_set_layout(col, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(col, LV_FLEX_FLOW_COLUMN);
    lv_obj_clear_flag(col, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(col, LV_OBJ_FLAG_CLICKABLE);

    /* Welcome card */
    lv_obj_t *c1 = make_card(col, "KNI ESP32-S3");
    lv_obj_t *ws = mk_label(c1,
        "Waveshare 27690 BSP v0.1\n"
        "ST7789 240x320 | LVGL 9.2\n"
        "CST328 capacitive touch",
        C_SUBTEXT, NULL);
    lv_label_set_long_mode(ws, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(ws, lv_pct(100));

    /* System status card */
    lv_obj_t *c2 = make_card(col, "SYSTEM STATUS");
    s_uptime_lbl = mk_label(c2, "Uptime     00:00:00", C_TEXT, NULL);
    s_heap_lbl   = mk_label(c2, "Free heap  --- KB",   C_TEXT, NULL);
    mk_label(c2, "CPU        240 MHz", C_SUBTEXT, NULL);

    /* Start live update */
    s_sys_timer = lv_timer_create(sys_timer_cb, 1000, NULL);
    sys_timer_cb(NULL);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/*  PANEL 1 — INFO                                                             */
/* ══════════════════════════════════════════════════════════════════════════ */

static void build_info_panel(lv_obj_t *scr)
{
    lv_obj_t *p = make_panel(scr);
    lv_obj_add_flag(p, LV_OBJ_FLAG_HIDDEN);
    s_panels[1] = p;

    lv_obj_t *card = make_card(p, "O PROJEKCIE");
    lv_obj_set_width(card, lv_pct(100));

    lv_obj_t *d = mk_label(card,
        "Kolo Naukowe Informatykow\n"
        "Uniwersytetu Rzeszowskiego\n\n"
        "Projekt BSP dla ESP32-S3\n"
        "z ekranem Waveshare 27690.\n\n"
        "SPI LCD ST7789  80 MHz\n"
        "I2C Touch CST328  400 kHz\n"
        "PSRAM 8 MB | Flash 16 MB\n\n"
        "Wersja BSP: v0.1.0",
        C_TEXT, NULL);
    lv_label_set_long_mode(d, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(d, lv_pct(100));
    lv_obj_set_style_text_line_space(d, 2, 0);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/*  PANEL 2 — SYS                                                              */
/* ══════════════════════════════════════════════════════════════════════════ */

static void build_sys_panel(lv_obj_t *scr)
{
    lv_obj_t *p = make_panel(scr);
    lv_obj_add_flag(p, LV_OBJ_FLAG_HIDDEN);
    s_panels[2] = p;

    static const struct { const char *k; const char *v; } specs[] = {
        { "SoC",          "ESP32-S3R8"     },
        { "CPU",          "Xtensa LX7 x2"  },
        { "Takt",         "240 MHz"         },
        { "Flash",        "16 MB QIO"       },
        { "PSRAM",        "8 MB Octal"      },
        { "LCD",          "ST7789T3 SPI"    },
        { "Rozdzielcz.",  "240x320"         },
        { "Dotyk",        "CST328 I2C"      },
        { "LVGL",         "9.2"             },
        { "IDF",          ">= 5.3"          },
    };
    int n = (int)(sizeof(specs) / sizeof(specs[0]));

    lv_obj_t *card = make_card(p, "HARDWARE");
    lv_obj_set_width(card, lv_pct(100));

    for (int i = 0; i < n; i++) {
        lv_obj_t *row = lv_obj_create(card);
        lv_obj_set_width(row,  lv_pct(100));
        lv_obj_set_height(row, LV_SIZE_CONTENT);
        lv_obj_set_style_bg_opa(row,      LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(row, 0,            0);
        lv_obj_set_style_pad_all(row,      2,            0);
        lv_obj_set_layout(row, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(row,
            LV_FLEX_ALIGN_SPACE_BETWEEN,
            LV_FLEX_ALIGN_CENTER,
            LV_FLEX_ALIGN_CENTER);
        lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_clear_flag(row, LV_OBJ_FLAG_CLICKABLE);

        mk_label(row, specs[i].k, C_SUBTEXT, NULL);
        mk_label(row, specs[i].v, C_TEXT,    NULL);
    }
}

/* ══════════════════════════════════════════════════════════════════════════ */
/*  MAIN SCREEN                                                                */
/* ══════════════════════════════════════════════════════════════════════════ */

static void build_main_screen(void)
{
    s_main = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(s_main, lv_color_hex(C_BG), 0);
    lv_obj_set_style_bg_opa(s_main, LV_OPA_COVER,         0);
    lv_obj_clear_flag(s_main, LV_OBJ_FLAG_SCROLLABLE);

    /* ── Top status bar 240×28 ──────────────────────────────────────────── */
    lv_obj_t *topbar = lv_obj_create(s_main);
    lv_obj_set_size(topbar, 240, 28);
    lv_obj_set_pos(topbar, 0, 0);
    lv_obj_set_style_bg_color(topbar,    lv_color_hex(C_BAR),    0);
    lv_obj_set_style_bg_opa(topbar,      LV_OPA_COVER,           0);
    lv_obj_set_style_border_color(topbar, lv_color_hex(C_TEAL),  0);
    lv_obj_set_style_border_side(topbar,  LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_border_width(topbar, 1,                     0);
    lv_obj_set_style_radius(topbar,       0,                     0);
    lv_obj_set_style_pad_hor(topbar,     10,                     0);
    lv_obj_set_style_pad_ver(topbar,      0,                     0);
    lv_obj_clear_flag(topbar, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *tl = mk_label(topbar, "KNI MAKERS", C_TEAL, NULL);
    lv_obj_set_align(tl, LV_ALIGN_LEFT_MID);

    lv_obj_t *tr = mk_label(topbar, "ESP32-S3", C_SUBTEXT, NULL);
    lv_obj_set_align(tr, LV_ALIGN_RIGHT_MID);

    /* ── Content panels ─────────────────────────────────────────────────── */
    build_home_panel(s_main);
    build_info_panel(s_main);
    build_sys_panel(s_main);

    /* ── Bottom menu bar 240×58 ─────────────────────────────────────────── */
    lv_obj_t *menu = lv_obj_create(s_main);
    lv_obj_set_size(menu, 240, 58);
    lv_obj_set_pos(menu, 0, 262);
    lv_obj_set_style_bg_color(menu,     lv_color_hex(C_BAR),   0);
    lv_obj_set_style_bg_opa(menu,       LV_OPA_COVER,          0);
    lv_obj_set_style_border_color(menu, lv_color_hex(C_TEAL),  0);
    lv_obj_set_style_border_side(menu,  LV_BORDER_SIDE_TOP,    0);
    lv_obj_set_style_border_width(menu, 1,                     0);
    lv_obj_set_style_radius(menu,       0,                     0);
    lv_obj_set_style_pad_all(menu,      0,                     0);
    lv_obj_clear_flag(menu, LV_OBJ_FLAG_SCROLLABLE);

    static const char *names[N_TABS] = { "HOME", "INFO", "SYS" };

    for (int i = 0; i < N_TABS; i++) {
        lv_obj_t *btn = lv_obj_create(menu);
        lv_obj_set_size(btn, 80, 58);
        lv_obj_set_pos(btn, i * 80, 0);
        lv_obj_set_style_bg_opa(btn,      LV_OPA_TRANSP, 0);
        lv_obj_set_style_bg_opa(btn,      LV_OPA_10,     LV_STATE_PRESSED);
        lv_obj_set_style_bg_color(btn,    lv_color_hex(C_TEAL), LV_STATE_PRESSED);
        lv_obj_set_style_border_width(btn, 0, 0);
        lv_obj_set_style_radius(btn,       0, 0);
        lv_obj_set_style_pad_all(btn,      0, 0);
        lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_event_cb(btn, on_tab_click, LV_EVENT_CLICKED,
                            (void *)(intptr_t)i);
        s_tab_btns[i] = btn;

        /* Active-tab top border indicator */
        if (i == 0) {
            lv_obj_set_style_border_color(btn, lv_color_hex(C_TEAL), 0);
            lv_obj_set_style_border_side(btn,  LV_BORDER_SIDE_TOP,   0);
            lv_obj_set_style_border_width(btn, 2,                    0);
        }

        lv_obj_t *lbl = mk_label(btn, names[i],
                                 i == 0 ? C_TEAL : C_INACTIVE, NULL);
        lv_obj_center(lbl);
        s_tab_lbls[i] = lbl;
    }
}

/* ══════════════════════════════════════════════════════════════════════════ */
/*  SPLASH SCREEN                                                              */
/* ══════════════════════════════════════════════════════════════════════════ */

static void bar_anim_cb(void *var, int32_t val)
{
    lv_obj_set_width((lv_obj_t *)var, val);
}

static void on_splash_done(lv_anim_t *a)
{
    (void)a;
    build_main_screen();
    lv_screen_load_anim(s_main, LV_SCR_LOAD_ANIM_FADE_IN, 500, 0, true);
}

static void build_splash_screen(void)
{
    s_splash = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(s_splash, lv_color_hex(C_BG), 0);
    lv_obj_set_style_bg_opa(s_splash,   LV_OPA_COVER,       0);
    lv_obj_set_style_pad_all(s_splash,  0,                  0);
    lv_obj_clear_flag(s_splash, LV_OBJ_FLAG_SCROLLABLE);

    /* ── Teal accent line at very top ──────────────────────────────────── */
    lv_obj_t *top_line = lv_obj_create(s_splash);
    lv_obj_set_size(top_line, 240, 2);
    lv_obj_set_pos(top_line, 0, 0);
    lv_obj_set_style_bg_color(top_line,    lv_color_hex(C_TEAL), 0);
    lv_obj_set_style_bg_opa(top_line,      LV_OPA_COVER,         0);
    lv_obj_set_style_border_width(top_line, 0,                   0);

    /* ── KNI logo image 160×160, centered horizontally ─────────────────── */
    lv_obj_t *logo_img = lv_image_create(s_splash);
    lv_image_set_src(logo_img, &kni_logo);
    lv_obj_align(logo_img, LV_ALIGN_TOP_MID, 0, 8);

    /* ── "KNI" large teal label ────────────────────────────────────────── */
    lv_obj_t *kni = mk_label(s_splash, "KNI", C_TEAL, &lv_font_montserrat_24);
    lv_obj_align(kni, LV_ALIGN_TOP_MID, 0, 176);

    /* ── Org names ─────────────────────────────────────────────────────── */
    lv_obj_t *o1 = mk_label(s_splash,
        "Kolo Naukowe Informatykow", C_TEXT, NULL);
    lv_obj_align(o1, LV_ALIGN_TOP_MID, 0, 210);

    lv_obj_t *o2 = mk_label(s_splash,
        "Uniwersytetu Rzeszowskiego", C_SUBTEXT, NULL);
    lv_obj_align(o2, LV_ALIGN_TOP_MID, 0, 228);

    /* ── Thin separator ────────────────────────────────────────────────── */
    lv_obj_t *sep = lv_obj_create(s_splash);
    lv_obj_set_size(sep, 140, 1);
    lv_obj_align(sep, LV_ALIGN_TOP_MID, 0, 252);
    lv_obj_set_style_bg_color(sep,    lv_color_hex(C_BORDER), 0);
    lv_obj_set_style_bg_opa(sep,      LV_OPA_COVER,           0);
    lv_obj_set_style_border_width(sep, 0,                     0);

    /* ── "Inicjalizacja..." label ────────────────────────────────────────  */
    lv_obj_t *init_lbl = mk_label(s_splash, "Inicjalizacja...", C_SUBTEXT, NULL);
    lv_obj_align(init_lbl, LV_ALIGN_TOP_MID, 0, 262);

    /* ── Loading bar background ─────────────────────────────────────────── */
    lv_obj_t *bar_bg = lv_obj_create(s_splash);
    lv_obj_set_size(bar_bg, 180, 4);
    lv_obj_align(bar_bg, LV_ALIGN_TOP_MID, 0, 284);
    lv_obj_set_style_bg_color(bar_bg,    lv_color_hex(C_BORDER), 0);
    lv_obj_set_style_bg_opa(bar_bg,      LV_OPA_COVER,           0);
    lv_obj_set_style_border_width(bar_bg, 0,                     0);
    lv_obj_set_style_radius(bar_bg,       2,                     0);
    lv_obj_set_style_pad_all(bar_bg,      0,                     0);
    lv_obj_clear_flag(bar_bg, LV_OBJ_FLAG_SCROLLABLE);

    /* ── Loading bar fill (animated) ──────────────────────────────────── */
    lv_obj_t *bar_fill = lv_obj_create(bar_bg);
    lv_obj_set_size(bar_fill, 0, 4);
    lv_obj_set_pos(bar_fill, 0, 0);
    lv_obj_set_style_bg_color(bar_fill,    lv_color_hex(C_TEAL), 0);
    lv_obj_set_style_bg_opa(bar_fill,      LV_OPA_COVER,         0);
    lv_obj_set_style_border_width(bar_fill, 0,                   0);
    lv_obj_set_style_radius(bar_fill,       2,                   0);

    /* ── Version label ────────────────────────────────────────────────── */
    lv_obj_t *ver = mk_label(s_splash, "v0.1.0", C_TEAL_DIM, NULL);
    lv_obj_align(ver, LV_ALIGN_BOTTOM_MID, 0, -8);

    /* ── Animate bar 0 → 180 px over 2.8 s, then switch screen ─────────── */
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, bar_fill);
    lv_anim_set_exec_cb(&a, bar_anim_cb);
    lv_anim_set_values(&a, 0, 180);
    lv_anim_set_duration(&a, 2800);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
    lv_anim_set_completed_cb(&a, on_splash_done);
    lv_anim_start(&a);

    lv_screen_load(s_splash);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/*  ENTRY POINT                                                                */
/* ══════════════════════════════════════════════════════════════════════════ */

void app_main(void)
{
    ESP_LOGI(TAG, "KNI ESP32-S3 boot");
    ESP_ERROR_CHECK(bsp_init());
    ESP_ERROR_CHECK(bsp_display_start());

    lv_display_t *disp  = NULL;
    lv_indev_t   *indev = NULL;
    ESP_ERROR_CHECK(bsp_lvgl_start(&disp, &indev));

    if (indev) {
        lv_indev_set_scroll_limit(indev, 3);   /* px before scroll starts — lower=snappier */
        lv_indev_set_scroll_throw(indev, 8);   /* % velocity lost/tick — lower=more momentum */
    }

    if (bsp_lvgl_lock(1000)) {
        build_splash_screen();
        bsp_lvgl_unlock();
    }

    ESP_LOGI(TAG, "running");
}
