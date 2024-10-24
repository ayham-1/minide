#include "minide/app.h"

#include "minide/file_manager.h"
#include "minide/font_manager.h"
#include "minide/fps_counter.h"
#include "minide/input.h"
#include "minide/texture_lender.h"

#include <GL/glew.h>
#include <unistd.h>

#include <unicode/putil.h>
#include <unicode/ulocdata.h>
#include <unicode/utypes.h>

static size_t nbFrames = 0;

int main(int argc, char * argv[])
{
	(void)argc;
	(void)argv;
#ifndef NDEBUG
	logger_init(DEBUG, "/tmp/.log", true);
#else
	logger_init(INFO, "/tmp/.log", true);
#endif

	UVersionInfo u_info;
	UErrorCode u_version_error_code = U_ZERO_ERROR;

	u_getVersion(u_info);
	log_info("ICU version: %i.%i.%i.%i", u_info[0], u_info[1], u_info[2], u_info[3]);

	ulocdata_getCLDRVersion(u_info, &u_version_error_code);
	if (U_FAILURE(u_version_error_code)) {
		log_error("ICU did not find unicode data");
		return -1;
	} else {
		log_info("ICU unicode data version: %i.%i.%i.%i", u_info[0], u_info[1], u_info[2], u_info[3]);
	}

	GLFWwindow * window;
	glfwSetErrorCallback(__glfw_error_callback);

	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	// windows should somehow be dynamically titled?
	// window = glfwCreateWindow(config.scr_width, config.scr_height, config.scr_title, NULL, NULL);
	// currently just use minide as title so that the wm can float it correctly
	window = glfwCreateWindow(app_config.scr_width, app_config.scr_height, "minide", NULL, NULL);
	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	input_man_init(window);

	glfwSetWindowSizeCallback(window, __glfw_size_callback);

	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	GLenum err = glewInit();
	if (GLEW_OK != err) {
		log_error("GLEW: %s", glewGetErrorString(err));
		return -1;
	} else {
		log_info("GLEW: version %s", glewGetString(GLEW_VERSION));
	}

#ifndef NDEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(__gl_callback, 0);
#endif

	file_manager_init();
	texture_lender_init(app_config.max_textures_available);
	fonts_man_init();

	glm_ortho(0.0f, (float)app_config.scr_width, (float)app_config.scr_height, 0, -1, 1, app_config.gl_projection);

	gl_wrapper_init();
	log_info("ran gl_wrapper_init");

	if (app_config.do_render_frame_ms)
		fps_counter_init(app_config.scr_width, app_config.scr_height);

	double lastSecondTime = glfwGetTime();
	while (!glfwWindowShouldClose(window)) {
		double frameStartTime = glfwGetTime();
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		gl_wrapper_render();

		if (app_config.do_render_frame_ms)
			fps_counter_render();

		glfwSwapBuffers(window);
		glfwPollEvents();

		if (app_config.scr_target_fps && glfwGetTime() < frameStartTime + 1.0 / app_config.scr_target_fps) {
			sleep(glfwGetTime() - (frameStartTime + 1.0 / app_config.scr_target_fps));
		}

		if (app_config.gl_wrapper_do_close)
			glfwSetWindowShouldClose(window, GLFW_TRUE);

		nbFrames++;
		if (glfwGetTime() - lastSecondTime >= 1) {
			if (app_config.do_print_frame_ms)
				log_info("fps: %d\t%f ms/frame", nbFrames, 1000.0f / nbFrames);
			if (app_config.do_render_frame_ms)
				fps_counter_update(nbFrames);
			nbFrames = 0;
			lastSecondTime = glfwGetTime();
		}
	}

	gl_wrapper_clean();

	log_info("closing %s...", app_config.scr_title);
	file_manager_cleanup();
	logger_cleanup();
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

void __glfw_error_callback(int error, const char * description)
{
	log_error("GLFW Error Callback: \t\nname:\t%s\n\tdescription:\t%s", error, description);
}

void __glfw_key_callback(GLFWwindow * window, int key, int scancode, int action, int mods)
{
	(void)window;
	(void)mods;
	(void)scancode;

	if (key == GLFW_KEY_Q && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);

	if (key == GLFW_KEY_Z && action == GLFW_PRESS)
		log_info("GLFW_KEY_Z");
}

void __glfw_size_callback(GLFWwindow * window, int width, int height)
{
	(void)window;

	app_config.scr_width = width;
	app_config.scr_height = height;

	glViewport(0, 0, width, height);

	glm_ortho(0.0f, (float)app_config.scr_width, (float)app_config.scr_height, 0, -1, 1, app_config.gl_projection);

	glfw_size_callback(width, height);
}

void __gl_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar * message,
		   const void * userParam)
{
	if (GL_DEBUG_TYPE_ERROR) {
		log_error("__gl_callback: %s type = 0x%x, severity = 0x%x, "
			  "message = %s\n",
			  "** GL ERROR **", type, severity, message);
		exit(1);
	} else {
		log_warn("__gl_callback: %s type = 0x%x, severity = 0x%x, "
			 "message = %s\n",
			 "", type, severity, message);
	}

	(void)source;
	(void)id;
	(void)length;
	(void)userParam;
}
