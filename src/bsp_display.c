#include "bsp/bsp.h"
#include "bsp_private.h"

#include "sdkconfig.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_commands.h"
#include "esp_log.h"
#include "esp_check.h"

static const char *TAG = "bsp.display";

static esp_lcd_panel_handle_t s_panel = NULL;
static esp_lcd_panel_io_handle_t s_io = NULL;

esp_err_t bsp_display_init_internal(esp_lcd_panel_handle_t *out_panel,
                                    esp_lcd_panel_io_handle_t *out_io)
{
    ESP_RETURN_ON_ERROR(bsp_spi_bus_init(), TAG, "SPI bus");

    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = CONFIG_BSP_LCD_PIN_DC,
        .cs_gpio_num = CONFIG_BSP_LCD_PIN_CS,
        .pclk_hz = CONFIG_BSP_LCD_SPI_HZ,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .spi_mode = 0,
        .trans_queue_depth = 10,
    };
    ESP_RETURN_ON_ERROR(
        esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)(intptr_t)CONFIG_BSP_LCD_SPI_HOST,
                                 &io_config, &s_io),
        TAG, "panel_io_spi");

    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = CONFIG_BSP_LCD_PIN_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 16,
    };
    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_st7789(s_io, &panel_config, &s_panel),
                        TAG, "new_panel_st7789");

    ESP_RETURN_ON_ERROR(esp_lcd_panel_reset(s_panel), TAG, "reset");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_init(s_panel), TAG, "init");

    /* Orientation. Adjust if the screen is upside-down in your enclosure. */
    esp_lcd_panel_invert_color(s_panel, true);
    esp_lcd_panel_mirror(s_panel, false, false);
    esp_lcd_panel_swap_xy(s_panel, false);
    esp_lcd_panel_set_gap(s_panel, 0, 0);

    ESP_RETURN_ON_ERROR(esp_lcd_panel_disp_on_off(s_panel, true), TAG, "disp_on");

    if (out_panel) *out_panel = s_panel;
    if (out_io) *out_io = s_io;
    return ESP_OK;
}

esp_err_t bsp_display_start(void)
{
    esp_lcd_panel_handle_t p;
    esp_lcd_panel_io_handle_t io;
    ESP_RETURN_ON_ERROR(bsp_display_init_internal(&p, &io), TAG, "display init");
    bsp_display_brightness_set(100);
    return ESP_OK;
}
