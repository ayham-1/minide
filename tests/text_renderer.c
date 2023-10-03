#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "../src/path.h"
#include "../src/logger.h"
#include "../src/text_renderer.h"

#define PATH "assets/FreeSans.ttf"
#define PATH_BYTES_NUM sizeof(PATH)

#include "../src/gl_wrapper.h"

int SCR_WIDTH = 800;
int SCR_HEIGHT = 600;
int SCR_TARGET_FPS = 0;
const char* SCR_TITLE = "test_text_renderer";
bool GL_WRAPPER_DO_CLOSE = false;

path_t p;
text_renderer_t renderer;

const char* str = "ABCDEFGHJKLMNOPQRSTVWXYZabcdefghjklmnopqrstvwxyz!@#$%^&*()_-=+[]{}\\|;:'\",.<>/?\0";

void gl_wrapper_init() {
    path_create(&p, PATH_BYTES_NUM);
    memcpy(p.fullPath.bytes, PATH, PATH_BYTES_NUM);

    text_renderer_init(&renderer, p, SCR_WIDTH, SCR_HEIGHT, 12);
}

void gl_wrapper_render() {
    text_renderer_line(&renderer, (byte_t*) str, 100, 100, 24);
    //GL_WRAPPER_DO_CLOSE = true;
}

void gl_wrapper_clean() {
    path_cleanup(&p);
    text_renderer_cleanup(&renderer);
}

void glfw_size_callback(int width, int height) {
   text_renderer_update_window_size(&renderer, width, height);
}
