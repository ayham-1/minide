cmake_minimum_required(VERSION 3.25.3)

project(
    minide
    VERSION 0.0.1
    LANGUAGES C
)

find_package(glfw3 3.3.8 REQUIRED)
find_package(glew REQUIRED)

add_executable(
    minide
    
    src/main.c
)

target_link_libraries(
    minide
    PRIVATE
    glfw
    ${GLEW_LIBRARIES}
)

set_property(TARGET minide PROPERTY CXX_STANDARD 23)
install(TARGETS minide)
