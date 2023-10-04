#include "fps_counter.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define PATH "assets/FreeSans.ttf"
#define PATH_BYTES_NUM sizeof(PATH)

static text_renderer_t renderer;
static size_t nbFrames;

void fps_counter_init(size_t width, size_t height) {
    path_t p;

    path_create(&p, PATH_BYTES_NUM);
    memcpy(p.fullPath.bytes, PATH, PATH_BYTES_NUM);
    
    text_renderer_init(&renderer, p, width, height, 14);
}

void fps_counter_render() {
    char str[21*2];
    sprintf(str, "%li fps | %f ms/frame", nbFrames, 1000.0f/nbFrames);

    text_renderer_line(&renderer, (byte_t*) &str, 0, 0);
}

void fps_counter_update(size_t frames) {
    nbFrames = frames;
}
