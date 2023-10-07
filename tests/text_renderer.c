#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "../src/path.h"
#include "../src/logger.h"
#include "../src/u8string.h"
#include "../src/text_renderer.h"

#define PATH "assets/FreeSans.ttf"
#define PATH_BYTES_NUM sizeof(PATH)

#include "../src/gl_wrapper.h"

GLsizei MAX_TEXTURES_AVIALABLE = 10;
int SCR_WIDTH = 800;
int SCR_HEIGHT = 600;
int SCR_TARGET_FPS = 15;
const char* SCR_TITLE = "test_text_renderer";
bool GL_WRAPPER_DO_CLOSE = false;

bool PRINT_FRAME_MS = false;
bool RENDER_FRAME_MS = true;

#define TEST_DATA "ABCDEFGHJKLMNOPQRSTVWXYZabcdefghjklmnopqrstvwxyz!@#$%^&*()_-=+[]{}\\|;:'\",.<>/?\0"

#define TEST_U8 "HELLO ᙭ WORLD 🌏"
#define TEST_U8_BYTES_NUM sizeof(TEST_U8)

char* str = NULL;
string_t u8str1;
string_t u8str2;

path_t p;
text_renderer_t renderer;

void gl_wrapper_init() {
    path_create(&p, PATH_BYTES_NUM);
    memcpy(p.fullPath.bytes, PATH, PATH_BYTES_NUM);

    text_renderer_init(&renderer, p, SCR_WIDTH, SCR_HEIGHT, 34);

    str = malloc(sizeof(TEST_DATA));

    u8str_create(&u8str1, TEST_U8_BYTES_NUM);
    memcpy(u8str1.bytes, TEST_U8, TEST_U8_BYTES_NUM);
    //u8encode enc = u8str_from_code_point((byte_t*)"U+4E3E\0\0");

    strcpy(str, TEST_DATA);
}

void gl_wrapper_render() {
    text_renderer_line(&renderer, (byte_t*) str, 100, 100);
    text_renderer_line(&renderer, u8str1.bytes, 100, 150);
    //hash_table_debug(&renderer.gcache.table, __glyph_cache_table_printer);
    GL_WRAPPER_DO_CLOSE = true;
}

void gl_wrapper_clean() {
    path_cleanup(&p);
    text_renderer_cleanup(&renderer);
    free(str);
}

void glfw_size_callback(int width, int height) {
   text_renderer_update_window_size(&renderer, width, height);
}
