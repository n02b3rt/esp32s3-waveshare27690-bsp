# Example project for the BSP.
# We pull in the BSP source (located 3 levels up) as a component by adding
# its parent dir to EXTRA_COMPONENT_DIRS. The folder name becomes the
# component name in CMake, so we keep consistency by renaming target in
# main/CMakeLists.txt REQUIRES.

cmake_minimum_required(VERSION 3.16)

# Point ESP-IDF at the BSP repo root's parent so it picks up the BSP folder.
get_filename_component(BSP_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
get_filename_component(BSP_PARENT "${BSP_ROOT}/.." ABSOLUTE)
list(APPEND EXTRA_COMPONENT_DIRS "${BSP_PARENT}")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(display_lvgl_demo)
