idf_component_register(
    SRCS "src/kni_ui.c"
    INCLUDE_DIRS "include"
    REQUIRES
        esp32s3-waveshare27690-bsp
        lvgl__lvgl
        esp_timer
        heap
)
