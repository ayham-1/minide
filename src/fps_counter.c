#include "fps_counter.h"

#define PATH "assets/FreeSans.ttf"
#define PATH_BYTES_NUM sizeof(PATH)

static text_renderer_t renderer;

void fps_counter_init() {
    path_t p;

    path_create(&p, PATH_BYTES_NUM);
    memcpy(p.fullPath.bytes, PATH, PATH_BYTES_NUM);
    
    text_renderer_init(&renderer, p, SCR_WIDTH, SCR_HEIGHT, 12);
}

void fps_counter_render(double ms) {
}
