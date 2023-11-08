#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "../src/path.h"
#include "../src/logger.h"
#include "../src/u8string.h"
#include "../src/fontconfig.h"

#include "../src/gl_wrapper.h"

GLsizei MAX_TEXTURES_AVIALABLE = 10;
int SCR_WIDTH = 800;
int SCR_HEIGHT = 600;
int SCR_TARGET_FPS = 1;
const char* SCR_TITLE = "test_fontconfig";
bool GL_WRAPPER_DO_CLOSE = false;

bool PRINT_FRAME_MS = false;
bool RENDER_FRAME_MS = false;

void gl_wrapper_init() {
    fc_init();
    log_debug("monospace: %s",
              fc_get_path_by_font(fc_request("monospace")));
    log_debug("serif: %s",
              fc_get_path_by_font(fc_request("serif")));
    log_debug("emoji: %s",
              fc_get_path_by_font(fc_request("emoji")));
}

void gl_wrapper_render() {
    GL_WRAPPER_DO_CLOSE = true;
}

void gl_wrapper_clean() {
    fc_clean();
}

void glfw_size_callback(int width, int height) {
}
