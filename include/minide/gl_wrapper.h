#ifndef GL_WRAPPER
#define GL_WRAPPER

#define GLFW_INCLUDE_NONE
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "logger.h"

typedef struct {
	GLsizei max_textures_available;
	int scr_width;
	int scr_height;
	int scr_target_fps;
	char * scr_title;

	bool do_print_frame_ms;
	bool do_render_frame_ms;

	bool gl_wrapper_do_close;
} gl_wrapper_config_t;

extern gl_wrapper_config_t config;

extern void gl_wrapper_init();
extern void gl_wrapper_render();
extern void gl_wrapper_clean();

extern void glfw_size_callback(int width, int height);

int main(int argc, char * argv[]);

void __glfw_error_callback(int error, const char * description);
void __glfw_key_callback(GLFWwindow * window, int key, int scancode, int action,
			 int mods);
void __glfw_size_callback(GLFWwindow * window, int width, int height);
void __gl_callback(GLenum source, GLenum type, GLuint id, GLenum severity,
		   GLsizei length, const GLchar * message,
		   const void * userParam);

#endif
