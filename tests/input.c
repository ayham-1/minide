#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "minide/input.h"
#include "minide/logger.h"

#include "minide/gl_wrapper.h"

gl_wrapper_config_t config = (gl_wrapper_config_t){
    .max_textures_available = 10,
    .scr_width = 1000,
    .scr_height = 700,
    .scr_target_fps = 1,
    .scr_title = "test_input",

    .do_print_frame_ms = false,
    .do_render_frame_ms = false,

    .gl_wrapper_do_close = false,
};

input_keys_hook_id key_id;
bool example_key_input_hook(int key, int scancode, int action, int mods)
{
	if (GLFW_KEY_Q == key && GLFW_PRESS == action)
		config.gl_wrapper_do_close = true;

	return false;
}

input_mouse_hook_id mouse_id;
bool example_mouse_input_hook(double xpos, double ypos)
{
	log_info("%f", xpos);
	log_info("%f", ypos);
	return false;
}

void gl_wrapper_init()
{
	key_id = input_man_register_keys_hook(example_key_input_hook);
	mouse_id = input_man_register_mouse_hook(example_mouse_input_hook);
}

void gl_wrapper_render() { config.gl_wrapper_do_close = true; }

void gl_wrapper_clean()
{
	input_man_unregister_keys_hook(key_id);
	input_man_unregister_mouse_hook(mouse_id);
}

void glfw_size_callback(int width, int height) {}
