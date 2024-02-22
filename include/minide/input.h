#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

#define GLFW_INCLUDE_NONE
#include <GL/glew.h>
#include <GLFW/glfw3.h>

typedef bool (*input_keys_hook)(int key, int scancode, int action, int mods);
typedef size_t input_keys_hook_id;

typedef bool (*input_mouse_hook)(double xpos, double ypos);
typedef size_t input_mouse_hook_id;

typedef struct {
	input_keys_hook * keys_hooks_list;
	bool * keys_hooks_registered_list;
	size_t keys_hooks_fullness;
	size_t keys_hooks_sz;

	input_mouse_hook * mouse_hooks_list;
	bool * mouse_hooks_registered_list;
	size_t mouse_hooks_fullness;
	size_t mouse_hooks_sz;

} input_man;

void input_man_init(GLFWwindow * window);

input_keys_hook_id input_man_register_keys_hook(input_keys_hook hook);
void input_man_unregister_keys_hook(input_keys_hook_id id);

input_mouse_hook_id input_man_register_mouse_hook(input_mouse_hook hook);
void input_man_unregister_mouse_hook(input_mouse_hook_id id);

void __input_glfw_key_callback(GLFWwindow * window, int key, int scancode, int action, int mods);
void __input_glfw_mouse_callback(GLFWwindow * window, double xpos, double ypos);

#endif
