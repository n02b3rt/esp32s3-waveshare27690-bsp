/*
 * Private / internal BSP state. Not a public API.
 */
#pragma once

#include "esp_err.h"
#include "esp_lcd_types.h"
#include "esp_lcd_touch.h"
#include "driver/i2c_master.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Lazily-initialized shared resources. */
esp_err_t bsp_spi_bus_init(void);   /* idempotent */
esp_err_t bsp_i2c_bus_init(void);   /* idempotent */

/* Display sub-module. Initializes ST7789 panel + backlight. */
esp_err_t bsp_display_init_internal(esp_lcd_panel_handle_t *out_panel,
                                    esp_lcd_panel_io_handle_t *out_io);

/* Touch sub-module. Initializes CST328 on I2C bus. */
esp_err_t bsp_touch_init_internal(esp_lcd_touch_handle_t *out_tp);

/* Shared I2C master bus handle (exposed for internal submodules only). */
i2c_master_bus_handle_t bsp_get_i2c_bus(void);

#ifdef __cplusplus
}
#endif
