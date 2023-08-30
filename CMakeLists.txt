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

set(SOURCES
    src/logger.c
    src/u8string.c
    src/path.c
)

add_library(
    min
    STATIC
    ${SOURCES}
)
target_link_libraries(
    min
    PRIVATE
    glfw
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
