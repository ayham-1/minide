#include <stdio.h>
#include <stdlib.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GL/glew.h>

#include "logger.h"

static void error_callback(int error, const char* description) {
    log_error("Error: %s\n", description);
}
 
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}
 
int main(int argc, char* argv[]) {
    printf("minide started\n");

    #ifndef NDEBUG
        logger_init(DEBUG, "/tmp/minide.log", true);
    #else
        logger_init(INFO, "/tmp/minide.log", true);
    #endif

    GLFWwindow* window;
    glfwSetErrorCallback(error_callback);
 
    if (!glfwInit())
        exit(EXIT_FAILURE);
 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
 
    window = glfwCreateWindow(640, 480, "minide", NULL, NULL);
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwSetKeyCallback(window, key_callback);
    glfwMakeContextCurrent(window);

    GLenum err = glewInit();
    if(GLEW_OK != err) {
        log_error("GLEW: %s", glewGetErrorString(err));
        return -1;
    } else {
        log_info("GLEW: version %s", glewGetString(GLEW_VERSION));
    }

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
 
    log_info("closing minide...");
    logger_cleanup();
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
    return 0;
}
