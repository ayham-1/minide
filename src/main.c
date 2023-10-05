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
const char* SCR_TITLE = "minide";
bool GL_WRAPPER_DO_CLOSE = false;

bool PRINT_FRAME_MS = false;
bool RENDER_FRAME_MS = false;

void gl_wrapper_init() {
}

void gl_wrapper_render() {
}

void gl_wrapper_clean() {
}

void glfw_size_callback(int width, int height) {
    (void)width;
    (void)height;
}
