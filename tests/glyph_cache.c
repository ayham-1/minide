#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "../src/path.h"
#include "../src/logger.h"
#include "../src/glyph_cache.h"

#define PATH "assets/FreeSans.ttf"
#define PATH_BYTES_NUM sizeof(PATH)

#include "../src/gl_wrapper.h"

GLsizei MAX_TEXTURES_AVIALABLE = 10;
int SCR_WIDTH = 800;
int SCR_HEIGHT = 600;
int SCR_TARGET_FPS = 1;
const char* SCR_TITLE = "test_glyph_cache";
bool GL_WRAPPER_DO_CLOSE = false;

bool PRINT_FRAME_MS = false;
bool RENDER_FRAME_MS = false;

path_t p;
glyph_cache cache;

void gl_wrapper_init() {
    path_create(&p, PATH_BYTES_NUM);
    memcpy(p.fullPath.bytes, PATH, PATH_BYTES_NUM);

    glyph_cache_init(&cache, p, 512, 24, true);
}

void gl_wrapper_render() {
    GL_WRAPPER_DO_CLOSE = true;
}

void gl_wrapper_clean() {
    path_cleanup(&p);
    glyph_cache_cleanup(&cache);
}

void glfw_size_callback(int width, int height) {
}
