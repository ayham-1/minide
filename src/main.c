#include <stdio.h>
#include <stdlib.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GL/glew.h>

#include <cglm/cglm.h>   /* for inline */
#include <cglm/call.h>   /* for library call (this also includes cglm.h) */

#include "logger.h"
#include "u8string.h"
#include "glyph_cache.h"

#define SCR_WIDTH 600
#define SCR_HEIGHT 800

const char* vertexShader = "\
#version 330 core\n\
layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>\n\
out vec2 TexCoords; \n\
uniform mat4 projection; \n\
void main() { \n\
gl_Position = projection * vec4(vertex.xy, 0.0, 1.0); \n\
TexCoords = vertex.zw; \n}";

const char* fragmentShader = "\
#version 330 core\n \
in vec2 TexCoords;\n \
out vec4 color;\n \
uniform sampler2D text;\n \
uniform vec3 textColor;\n \
void main() {\n \
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);\n \
    color = vec4(textColor, 1.0) * sampled; }";

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
 
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "minide", NULL, NULL);
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

    /* TODO(ayham): freetype TESTING  */

    unsigned int vertex, fragment;
    int success;
    char infoLog[512];

    // vertex Shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertexShader, NULL);
    glCompileShader(vertex);
    // print compile errors if any
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        log_error("glsl: vertex compile failed. error: %s", infoLog);
    };
    // fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragmentShader, NULL);
    glCompileShader(fragment);
    // print compile errors if any
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        log_error("glsl: fragment compile failed. error: %s", infoLog);
        return -1;
    };

    unsigned int glslID = glCreateProgram();
    glAttachShader(glslID, vertex);
    glAttachShader(glslID, fragment);
    glLinkProgram(glslID);
    // print linking errors if any
    glGetProgramiv(glslID, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(glslID, 512, NULL, infoLog);
        log_error("glsl: program link failed. error: %s", infoLog);
    }

    mat4 projection;
    glm_ortho(0.0f, SCR_WIDTH, 0.0f, SCR_HEIGHT, 0.0, 1.0, projection);
    glUseProgram(glslID);
    glUniformMatrix4fv(glGetUniformLocation(glslID, "projection"), 1, GL_FALSE, (float*)projection);

    path_t font;
    #define FONT "assets/FreeSans.ttf"
    path_create(&font, sizeof(FONT)+1);
    memcpy(font.fullPath.bytes, FONT, sizeof(FONT));
    
//    // generate texture
//    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
//    unsigned int texture;
//    glGenTextures(1, &texture);
//    glBindTexture(GL_TEXTURE_2D, texture);
//    glTexImage2D(
//        GL_TEXTURE_2D,
//        0,
//        GL_RED,
//        face->glyph->bitmap.width,
//        face->glyph->bitmap.rows,
//        0,
//        GL_RED,
//        GL_UNSIGNED_BYTE,
//        face->glyph->bitmap.buffer
//    );
//    // set texture options
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
//
//    glEnable(GL_BLEND);
//    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//
//    unsigned int VAO, VBO;
//    glGenVertexArrays(1, &VAO);
//    glGenBuffers(1, &VBO);
//    glBindVertexArray(VAO);
//    glBindBuffer(GL_ARRAY_BUFFER, VBO);
//    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
//    glEnableVertexAttribArray(0);
//    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
//    glBindBuffer(GL_ARRAY_BUFFER, 0);
//
//    glUniform3f(glGetUniformLocation(glslID, "textColor"), 1.0, 1.0, 1.0);
//    glActiveTexture(GL_TEXTURE0);
//    glBindTexture(GL_TEXTURE_2D, texture);
//    glBindBuffer(GL_ARRAY_BUFFER, VBO);
//    float xpos = 50;
//    float ypos = 50 - face->glyph->bitmap.rows + face->glyph->bitmap_top;
//
//    float w = face->glyph->bitmap.width;
//    float h = face->glyph->bitmap.rows;
//
//    float vertices[6][4] = {
//        { xpos,     ypos + h,   0.0f, 0.0f },            
//        { xpos,     ypos,       0.0f, 1.0f },
//        { xpos + w, ypos,       1.0f, 1.0f },
//
//        { xpos,     ypos + h,   0.0f, 0.0f },
//        { xpos + w, ypos,       1.0f, 1.0f },
//        { xpos + w, ypos + h,   1.0f, 0.0f }           
//    };
//    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData
//
    //
    //FT_Done_Face(face);
    //FT_Done_FreeType(library);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        //glDrawArrays(GL_TRIANGLES, 0, 6);
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
