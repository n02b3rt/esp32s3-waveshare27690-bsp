#include "bsp/bsp.h"
#include "bsp_private.h"

#include "sdkconfig.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_touch.h"
#include "esp_lcd_touch_cst328.h"
#include "esp_log.h"
#include "esp_check.h"

static const char *TAG = "bsp.touch";

esp_err_t bsp_touch_init_internal(esp_lcd_touch_handle_t *out_tp)
{
    ESP_RETURN_ON_ERROR(bsp_i2c_bus_init(), TAG, "I2C bus");

    esp_lcd_panel_io_handle_t tp_io_handle = NULL;
    esp_lcd_panel_io_i2c_config_t tp_io_config = ESP_LCD_TOUCH_IO_I2C_CST328_CONFIG();
    tp_io_config.scl_speed_hz = CONFIG_BSP_TOUCH_I2C_HZ;

    ESP_RETURN_ON_ERROR(
        esp_lcd_new_panel_io_i2c(bsp_get_i2c_bus(), &tp_io_config, &tp_io_handle),
        TAG, "new_panel_io_i2c");

    esp_lcd_touch_config_t tp_cfg = {
        .x_max = BSP_LCD_H_RES,
        .y_max = BSP_LCD_V_RES,
        .rst_gpio_num = CONFIG_BSP_TOUCH_PIN_RST,
        .int_gpio_num = CONFIG_BSP_TOUCH_PIN_INT,
        .levels = {
            .reset = 0,
            .interrupt = 0,
        },
        .flags = {
            .swap_xy = 0,
            .mirror_x = 0,
            .mirror_y = 0,
        },
    };

    esp_lcd_touch_handle_t tp = NULL;
    ESP_RETURN_ON_ERROR(esp_lcd_touch_new_i2c_cst328(tp_io_handle, &tp_cfg, &tp),
                        TAG, "new_i2c_cst328");

    if (out_tp) *out_tp = tp;
    return ESP_OK;
}
