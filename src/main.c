#include <stdio.h>
#include <stdlib.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GL/glew.h>

#include "logger.h"
#include "u8string.h"
#include "glyph_cache.h"

#include "gl_wrapper.h"

int SCR_WIDTH = 800;
int SCR_HEIGHT = 600;
int SCR_TARGET_FPS = 30;
const char* SCR_TITLE = "minide";
bool GL_WRAPPER_DO_CLOSE = false;

void gl_wrapper_init() {
}

void gl_wrapper_render() {
}

void gl_wrapper_clean() {

}
