cmake_minimum_required(VERSION 3.25.3)

option(BUILD_TESTS "Build test programs" OFF)

project(
    minide
    VERSION 0.0.1
    LANGUAGES C
)
#set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

find_package(glfw3 3.3.8 REQUIRED)
find_package(glew REQUIRED)
find_package(cglm REQUIRED)
find_package(Freetype REQUIRED)
find_package(harfbuzz REQUIRED)
find_package(ICU COMPONENTS data uc i18n io REQUIRED)
find_package(Fontconfig REQUIRED)


find_package(PkgConfig REQUIRED)
pkg_check_modules(glib REQUIRED IMPORTED_TARGET glib-2.0)
pkg_check_modules(cairo REQUIRED IMPORTED_TARGET cairo)
pkg_check_modules(rsvg REQUIRED IMPORTED_TARGET librsvg-2.0)

set(SOURCES
    src/gl_wrapper.c
    src/texture_lender.c

    src/logger.c
    src/u8string.c
    src/path.c

    src/types/hash_table.c

    src/glyph_cache.c
    src/fontconfig.c
    src/font.c
    src/font_manager.c
    src/shaper.c

    src/shaders_util.c
    src/text_renderer.c

    src/fps_counter.c
)

add_library(
    min
    STATIC
    ${SOURCES}
)
target_link_libraries(
    min
    PUBLIC
    glfw
    cglm
    freetype
    harfbuzz
    fontconfig
    PkgConfig::glib
    PkgConfig::cairo
    PkgConfig::rsvg

    ${ICU_LIBRARIES}
    ${GLEW_LIBRARIES}
)
set_property(TARGET min PROPERTY C_STANDARD 23)
target_compile_options(min PRIVATE -Wall -Wextra -Wpedantic)

add_executable(
    minide
    src/main.c
)
target_link_libraries(
    minide
    PRIVATE
    min
    glfw
    ${GLEW_LIBRARIES}
)
set_property(TARGET minide PROPERTY C_STANDARD 23)
target_compile_options(minide PRIVATE -Wall -Wextra -Wpedantic)

if (BUILD_TESTS)
    add_subdirectory(tests)
endif (BUILD_TESTS)

install(TARGETS minide)
