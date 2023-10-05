#include "gl_wrapper.h"

#include "fps_counter.h"
#include "texture_lender.h"

#include <unistd.h>

static size_t nbFrames = 0;

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    #ifndef NDEBUG
        logger_init(DEBUG, "/tmp/.log", true);
    #else
        logger_init(INFO, "/tmp/.log", true);
    #endif

    GLFWwindow* window;
    glfwSetErrorCallback(__glfw_error_callback);
 
    if (!glfwInit())
        exit(EXIT_FAILURE);
 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, SCR_TITLE, NULL, NULL);
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, __glfw_key_callback);
    glfwSetWindowSizeCallback(window, __glfw_size_callback);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);

    GLenum err = glewInit();
    if(GLEW_OK != err) {
        log_error("GLEW: %s", glewGetErrorString(err));
        return -1;
    } else {
        log_info("GLEW: version %s", glewGetString(GLEW_VERSION));
    }
    texture_lender_init(MAX_TEXTURES_AVIALABLE);

    gl_wrapper_init();
    log_info("ran gl_wrapper_init");


    if (RENDER_FRAME_MS)
        fps_counter_init(SCR_WIDTH, SCR_HEIGHT);

    double lastSecondTime = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        double frameStartTime = glfwGetTime();
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        gl_wrapper_render();

        if (RENDER_FRAME_MS)
            fps_counter_render();

        glfwSwapBuffers(window);
        glfwPollEvents();

        while (SCR_TARGET_FPS && glfwGetTime() < frameStartTime + 1.0 / SCR_TARGET_FPS) {
            //sleep(glfwGetTime() - (lastTime + 1.0 / SCR_TARGET_FPS));
        }

        if (GL_WRAPPER_DO_CLOSE) glfwSetWindowShouldClose(window, GLFW_TRUE);

        nbFrames++;
        if (glfwGetTime() - lastSecondTime >= 1) {
            if (PRINT_FRAME_MS)
                log_info("fps: %d\t%f ms/frame", nbFrames, 1000.0f/nbFrames);
            if (RENDER_FRAME_MS)
                fps_counter_update(nbFrames);
            nbFrames = 0;
            lastSecondTime = glfwGetTime();
        }
    }

    gl_wrapper_clean();

    log_info("closing %s...", SCR_TITLE);
    logger_cleanup();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

void __glfw_error_callback(int error, const char* description) {
    log_error("GLFW Error Callback: \t\nname:\t%s\n\tdescription:\t%s", error, description);
}
 
void __glfw_key_callback(GLFWwindow* window, 
                         int key, int scancode, int action, int mods) {
    (void)window;
    (void)mods;
    (void)scancode;

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void __glfw_size_callback(GLFWwindow* window, 
                                 int width, int height) {
    (void)window;

    SCR_WIDTH = width;
    SCR_HEIGHT = height;

    glViewport(0, 0, width, height);

    glfw_size_callback(width, height);
}