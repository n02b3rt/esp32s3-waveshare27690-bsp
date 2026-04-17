#include "bsp/bsp.h"
#include "bsp_private.h"

#include "sdkconfig.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "esp_check.h"
#include "esp_log.h"

static const char *TAG = "bsp";

static bool s_spi_ready = false;
static bool s_i2c_ready = false;
static i2c_master_bus_handle_t s_i2c_bus = NULL;

esp_err_t bsp_spi_bus_init(void)
{
    if (s_spi_ready) {
        return ESP_OK;
    }

    spi_bus_config_t buscfg = {
        .sclk_io_num = CONFIG_BSP_LCD_PIN_SCLK,
        .mosi_io_num = CONFIG_BSP_LCD_PIN_MOSI,
        .miso_io_num = -1,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = BSP_LCD_H_RES * BSP_LCD_V_RES * sizeof(uint16_t),
    };
    ESP_RETURN_ON_ERROR(
        spi_bus_initialize((spi_host_device_t)CONFIG_BSP_LCD_SPI_HOST,
                           &buscfg, SPI_DMA_CH_AUTO),
        TAG, "spi_bus_initialize failed");

    s_spi_ready = true;
    return ESP_OK;
}

esp_err_t bsp_i2c_bus_init(void)
{
    if (s_i2c_ready) {
        return ESP_OK;
    }

    i2c_master_bus_config_t bus_config = {
        .i2c_port = CONFIG_BSP_TOUCH_I2C_PORT,
        .sda_io_num = CONFIG_BSP_TOUCH_PIN_SDA,
        .scl_io_num = CONFIG_BSP_TOUCH_PIN_SCL,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    ESP_RETURN_ON_ERROR(i2c_new_master_bus(&bus_config, &s_i2c_bus),
                        TAG, "i2c_new_master_bus failed");

    s_i2c_ready = true;
    return ESP_OK;
}

i2c_master_bus_handle_t bsp_get_i2c_bus(void)
{
    return s_i2c_bus;
}

esp_err_t bsp_init(void)
{
    ESP_LOGI(TAG, "init: ESP32-S3-Touch-LCD-2.8 (Waveshare 27690)");
    ESP_RETURN_ON_ERROR(bsp_spi_bus_init(), TAG, "SPI bus init");
    ESP_RETURN_ON_ERROR(bsp_i2c_bus_init(), TAG, "I2C bus init");

    /* Backlight GPIO -> off by default, enabled in bsp_display_start(). */
    gpio_config_t bl = {
        .pin_bit_mask = 1ULL << CONFIG_BSP_LCD_PIN_BL,
        .mode = GPIO_MODE_OUTPUT,
    };
    ESP_RETURN_ON_ERROR(gpio_config(&bl), TAG, "backlight gpio");
    gpio_set_level(CONFIG_BSP_LCD_PIN_BL, CONFIG_BSP_LCD_BL_ACTIVE_HIGH ? 0 : 1);

    return ESP_OK;
}

esp_err_t bsp_display_brightness_set(uint8_t brightness)
{
    int on = (brightness > 0) ? 1 : 0;
    if (!CONFIG_BSP_LCD_BL_ACTIVE_HIGH) on = !on;
    gpio_set_level(CONFIG_BSP_LCD_PIN_BL, on);
    return ESP_OK;
}
