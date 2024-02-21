#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

#define GLFW_INCLUDE_NONE
#include <GL/glew.h>
#include <GLFW/glfw3.h>

typedef void (*input_hook)(int key, int scancode, int action, int mods);

typedef struct {
	input_hook * hooks_list;
	size_t hooks_capacity;
	size_t hooks_cnt;
} input_man;

size_t input_man_register_hook();

void __input_glfw_key_callback(GLFWwindow * window, int key, int scancode, int action, int mods);

#endif
