#pragma once
#include "esp_err.h"
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Color palette ─────────────────────────────────────────────────────────
 * Each token has its own #ifndef so you can override any single color
 * without redefining the whole palette:
 *
 *   #define KNI_UI_C_TEAL  0xFF6600   // swap accent to orange
 *   #include "kni_ui/kni_ui.h"
 *
 * Or pass via compiler flag: -DKNI_UI_C_TEAL=0xFF6600
 * ─────────────────────────────────────────────────────────────────────────── */
#ifndef KNI_UI_C_BG
#define KNI_UI_C_BG       0x07111A   /* screen background  */
#endif
#ifndef KNI_UI_C_CARD
#define KNI_UI_C_CARD     0x16263A   /* card surface       */
#endif
#ifndef KNI_UI_C_TEAL
#define KNI_UI_C_TEAL     0x00CCA3   /* accent / active    */
#endif
#ifndef KNI_UI_C_TEAL_DIM
#define KNI_UI_C_TEAL_DIM 0x00856B   /* dimmed accent      */
#endif
#ifndef KNI_UI_C_TEXT
#define KNI_UI_C_TEXT     0xDDECF8   /* primary text       */
#endif
#ifndef KNI_UI_C_SUBTEXT
#define KNI_UI_C_SUBTEXT  0x6888A8   /* secondary text     */
#endif
#ifndef KNI_UI_C_BORDER
#define KNI_UI_C_BORDER   0x1D3148   /* card border        */
#endif
#ifndef KNI_UI_C_BAR
#define KNI_UI_C_BAR      0x0B1825   /* top / bottom bars  */
#endif
#ifndef KNI_UI_C_INACTIVE
#define KNI_UI_C_INACTIVE 0x3D5A78   /* inactive tab label */
#endif

#define KNI_UI_MAX_TABS   5

/* ── Tab descriptor ─────────────────────────────────────────────────────
 *  build() is called once when the main screen is created.
 *  The panel passed in is a scrollable container — add cards to it.
 * ─────────────────────────────────────────────────────────────────────── */
typedef struct {
    const char *label;                    /* menu bar text, short, uppercase */
    void       (*build)(lv_obj_t *panel); /* content builder callback        */
} kni_ui_tab_t;

/* ── Top-level config ───────────────────────────────────────────────────── */
typedef struct {
    const char           *title;       /* top bar left  — e.g. "KNI" */
    const char           *subtitle;    /* top bar right — e.g. "ESP32-S3"   */
    const lv_image_dsc_t *logo;        /* splash logo image (NULL = skip)    */
    uint32_t              splash_ms;   /* animated loading bar duration (ms) */
    const kni_ui_tab_t   *tabs;
    int                   tab_count;   /* 1–KNI_UI_MAX_TABS                  */
} kni_ui_cfg_t;

/* ── Lifecycle ──────────────────────────────────────────────────────────
 *  Call from within bsp_lvgl_lock() / bsp_lvgl_unlock().
 *  Shows splash screen, then fades in the main tabbed screen.
 * ─────────────────────────────────────────────────────────────────────── */
esp_err_t kni_ui_start(const kni_ui_cfg_t *cfg);

/* ── Content helpers — call from inside tab build() callbacks ───────────
 *
 *  Typical pattern:
 *
 *    static void build_home(lv_obj_t *panel) {
 *        lv_obj_t *card = kni_ui_card(panel, "STATUS");
 *        lv_obj_t *val  = kni_ui_kv_row(card, "Uptime", "00:00:00");
 *        // keep `val` pointer — update via lv_label_set_text_fmt() later
 *    }
 * ─────────────────────────────────────────────────────────────────────── */

/* Create a styled card with a teal title. Returns the card object. */
lv_obj_t *kni_ui_card(lv_obj_t *panel, const char *title);

/**
 * Add a key–value row to a card.
 * Returns the VALUE label so callers can update it (e.g. lv_label_set_text_fmt).
 */
lv_obj_t *kni_ui_kv_row(lv_obj_t *card, const char *key, const char *value);

/* Add a wrapped text paragraph inside a card. Returns the label. */
lv_obj_t *kni_ui_text(lv_obj_t *card, const char *text, uint32_t color);

/**
 * Create a transparent flex-column wrapper inside a panel.
 * Use this when a tab needs more than one stacked card.
 * Pass the returned wrapper instead of panel to kni_ui_card().
 */
lv_obj_t *kni_ui_col(lv_obj_t *panel);

#ifdef __cplusplus
}
#endif
