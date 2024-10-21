#include "minide/input.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>


input_man __input_man;

void input_man_init(GLFWwindow * window)
{
	glfwSetKeyCallback(window, __input_glfw_key_callback);
	glfwSetCursorPosCallback(window, __input_glfw_mouse_callback);

	// keys
	__input_man.keys_hooks_sz = 3;
	__input_man.keys_hooks_fullness = 0;

	__input_man.keys_hooks_list = malloc(__input_man.keys_hooks_sz * sizeof(input_keys_hook));
	__input_man.keys_hooks_registered_list = malloc(__input_man.keys_hooks_sz * sizeof(bool));
	memset(__input_man.keys_hooks_registered_list, false, __input_man.keys_hooks_sz);

	// mouse
	__input_man.mouse_hooks_sz = 3;
	__input_man.mouse_hooks_fullness = 0;

	__input_man.mouse_hooks_list = malloc(__input_man.mouse_hooks_sz * sizeof(input_mouse_hook));
	__input_man.mouse_hooks_registered_list = malloc(__input_man.mouse_hooks_sz * sizeof(bool));
	memset(__input_man.mouse_hooks_registered_list, false, __input_man.mouse_hooks_sz);
}

input_keys_hook_id input_man_register_keys_hook(input_keys_hook hook)
{
	assert(hook);

	if (__input_man.keys_hooks_fullness + 1 >= __input_man.keys_hooks_sz) {
		__input_man.keys_hooks_sz++;
		__input_man.keys_hooks_sz *= 1.5;

		__input_man.keys_hooks_list =
		    realloc(__input_man.keys_hooks_list, __input_man.keys_hooks_sz * sizeof(input_keys_hook));
		__input_man.keys_hooks_registered_list =
		    realloc(__input_man.keys_hooks_list, __input_man.keys_hooks_sz * sizeof(bool));
	}

	for (size_t i = 0; i < __input_man.keys_hooks_sz; i++) {
		if (false == __input_man.keys_hooks_registered_list[i]) {
			__input_man.keys_hooks_list[i] = hook;
			__input_man.keys_hooks_registered_list[i] = true;
			__input_man.keys_hooks_fullness++;
			return i;
		}
	}

	return -1; // ???
}

void input_man_unregister_keys_hook(input_keys_hook_id id)
{
	assert(id >= 0);
	assert(id < __input_man.keys_hooks_sz);
	assert(__input_man.keys_hooks_registered_list[id]);

	__input_man.keys_hooks_registered_list[id] = false;
	__input_man.keys_hooks_list[id] = NULL;
	__input_man.keys_hooks_fullness--;
}

input_mouse_hook_id input_man_register_mouse_hook(input_mouse_hook hook)
{
	assert(hook);

	if (__input_man.mouse_hooks_fullness + 1 >= __input_man.mouse_hooks_sz) {
		__input_man.mouse_hooks_sz++;
		__input_man.mouse_hooks_sz *= 1.5;

		__input_man.mouse_hooks_list =
		    realloc(__input_man.mouse_hooks_list, __input_man.mouse_hooks_sz * sizeof(input_mouse_hook));
		__input_man.mouse_hooks_registered_list =
		    realloc(__input_man.mouse_hooks_list, __input_man.mouse_hooks_sz * sizeof(bool));
	}

	for (size_t i = 0; i < __input_man.mouse_hooks_sz; i++) {
		if (false == __input_man.mouse_hooks_registered_list[i]) {
			__input_man.mouse_hooks_list[i] = hook;
			__input_man.mouse_hooks_registered_list[i] = true;
			__input_man.mouse_hooks_fullness++;
			return i;
		}
	}

	return -1; // ???
}

void input_man_unregister_mouse_hook(input_mouse_hook_id id)
{
	assert(id >= 0);
	assert(id < __input_man.mouse_hooks_sz);
	assert(__input_man.mouse_hooks_registered_list[id]);

	__input_man.mouse_hooks_registered_list[id] = false;
	__input_man.mouse_hooks_list[id] = NULL;
	__input_man.mouse_hooks_fullness--;
}

void __input_glfw_key_callback(GLFWwindow * window, int key, int scancode, int action, int mods)
{
	for (size_t i = 0; i < __input_man.keys_hooks_fullness; i++) {
		if (__input_man.keys_hooks_registered_list[i] && __input_man.keys_hooks_list[i]) {
			if (__input_man.keys_hooks_list[i](key, scancode, action, mods)) {
				break; // event captured
			}
		}
	}
}

void __input_glfw_mouse_callback(GLFWwindow * window, double xpos, double ypos)
{
	for (size_t i = 0; i < __input_man.mouse_hooks_fullness; i++) {
		if (__input_man.mouse_hooks_registered_list[i] && __input_man.mouse_hooks_list[i]) {
			if (__input_man.mouse_hooks_list[i](xpos, ypos)) {
				break; // event captured
			}
		}
	}
}
