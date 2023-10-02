#ifndef GL_WRAPPER
#define GL_WRAPPER

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GL/glew.h>

#include "logger.h"

extern int SCR_WIDTH;
extern int SCR_HEIGHT;
extern int SCR_TARGET_FPS;
extern const char* SCR_TITLE;
extern bool GL_WRAPPER_DO_CLOSE;

extern void gl_wrapper_init();
extern void gl_wrapper_render();
extern void gl_wrapper_clean();

int main(int argc, char* argv[]);

static void __glfw_error_callback(int error, const char* description);
static void __glfw_key_callback(GLFWwindow* window, 
                         int key, int scancode, int action, int mods);

#endif
