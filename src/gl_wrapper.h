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

extern bool PRINT_FRAME_MS;
extern bool RENDER_FRAME_MS;

extern void gl_wrapper_init();
extern void gl_wrapper_render();
extern void gl_wrapper_clean();

extern void glfw_size_callback(int width, int height);

int main(int argc, char* argv[]);

void __glfw_error_callback(int error, const char* description);
void __glfw_key_callback(GLFWwindow* window, 
                         int key, int scancode, int action, int mods);
void __glfw_size_callback(GLFWwindow* window, 
                                 int width, int height);

#endif
