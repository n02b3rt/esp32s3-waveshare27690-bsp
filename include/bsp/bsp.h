/*
 * Waveshare ESP32-S3-Touch-LCD-2.8 (27690) BSP
 * Public API.
 *
 * Usage (typical):
 *   bsp_init();                          // I2C/SPI/power
 *   bsp_display_start();                 // turn on LCD + backlight
 *   lv_display_t  *disp;
 *   lv_indev_t    *indev;
 *   bsp_lvgl_start(&disp, &indev);       // boots LVGL task + attaches display+touch
 *
 *   bsp_lvgl_lock(portMAX_DELAY);
 *   // ... your LVGL code ...
 *   bsp_lvgl_unlock();
 */

#pragma once

#include "esp_err.h"
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Display resolution (hardware native). */
#define BSP_LCD_H_RES  (240)
#define BSP_LCD_V_RES  (320)

/**
 * @brief One-shot board init: SPI bus, I2C bus, backlight GPIO.
 *        Call once at boot before any other bsp_* function.
 */
esp_err_t bsp_init(void);

/**
 * @brief Turn on LCD panel + backlight. Does not start LVGL.
 */
esp_err_t bsp_display_start(void);

/**
 * @brief Set backlight brightness. brightness 0..100.
 *        Currently implemented as on/off (0 = off, >0 = on).
 *        PWM TODO.
 */
esp_err_t bsp_display_brightness_set(uint8_t brightness);

/**
 * @brief Start LVGL task and attach display + touch as an LVGL input device.
 *        Must be called after bsp_init() + bsp_display_start().
 *
 * @param[out] disp   Optional output: LVGL display handle.
 * @param[out] indev  Optional output: LVGL touch input device handle.
 */
esp_err_t bsp_lvgl_start(lv_display_t **disp, lv_indev_t **indev);

/**
 * @brief Acquire LVGL mutex. Must be held around any lv_* call from outside
 *        the LVGL task.
 */
bool bsp_lvgl_lock(int timeout_ms);

/**
 * @brief Release LVGL mutex.
 */
void bsp_lvgl_unlock(void);

#ifdef __cplusplus
}
#endif
