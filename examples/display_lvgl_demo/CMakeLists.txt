# Example project for the BSP.
# BSP_PARENT finds esp32s3-waveshare27690-bsp via EXTRA_COMPONENT_DIRS.
# kni_ui (inside the BSP repo) is referenced via main/idf_component.yml local path.

cmake_minimum_required(VERSION 3.16)

get_filename_component(BSP_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
get_filename_component(BSP_PARENT "${BSP_ROOT}/.." ABSOLUTE)
list(APPEND EXTRA_COMPONENT_DIRS "${BSP_PARENT}")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(display_lvgl_demo)
