#include "shaders_util.h"
#include "logger.h"

#include <GL/glew.h>
#include <stdio.h>
#include <stdlib.h>

/* URL:
 * https://gitlab.com/wikibooks-opengl/modern-tutorials/-/blame/master/common/shader_utils.cpp#L15
 */
char *file_read(const char *filename) {
    FILE *in = fopen(filename, "rb");

    if (in == NULL)
        return NULL;

    int res_size = BUFSIZ;
    char *res = (char *)malloc(res_size);
    int nb_read_total = 0;

    while (!feof(in) && !ferror(in)) {
        if (nb_read_total + BUFSIZ > res_size) {
            if (res_size > 10 * 1024 * 1024)
                break;
            res_size = res_size * 2;
            res = (char *)realloc(res, res_size);
        }
        char *p_res = res + nb_read_total;
        nb_read_total += fread(p_res, 1, BUFSIZ, in);
    }

    fclose(in);
    res = (char *)realloc(res, nb_read_total + 1);
    res[nb_read_total] = '\0';
    return res;
}

void print_shader_log(GLuint object) {
    GLint log_length = 0;
    if (glIsShader(object))
        glGetShaderiv(object, GL_INFO_LOG_LENGTH, &log_length);
    else if (glIsProgram(object))
        glGetProgramiv(object, GL_INFO_LOG_LENGTH, &log_length);
    else {
        log_error("not a shader or a program\n");
        return;
    }

    char *log = (char *)malloc(log_length);

    if (glIsShader(object))
        glGetShaderInfoLog(object, log_length, NULL, log);
    else if (glIsProgram(object))
        glGetProgramInfoLog(object, log_length, NULL, log);

    log_info("%s", log);
    free(log);
}

GLuint shader_create(const char *filename, GLenum type) {
    const GLchar *source = file_read(filename);

    if (source == NULL) {
        log_error("error opening shader file %s", filename);
        exit(1);
    }

    GLuint res = glCreateShader(type);

    glShaderSource(res, 1, &source, NULL);
    free((void *)source);

    glCompileShader(res);
    GLint compile_ok = GL_FALSE;
    glGetShaderiv(res, GL_COMPILE_STATUS, &compile_ok);
    if (compile_ok == GL_FALSE) {
        log_error("failed compiling shader: %s", filename);
        print_shader_log(res);
        exit(1);
        return 0;
    }

    return res;
}

GLuint shader_program_create(const char *vertexfile, const char *fragmentfile) {
    GLuint program = glCreateProgram();
    GLuint shader;

    if (vertexfile) {
        shader = shader_create(vertexfile, GL_VERTEX_SHADER);
        if (!shader)
            return 0;
        glAttachShader(program, shader);
    }

    if (fragmentfile) {
        shader = shader_create(fragmentfile, GL_FRAGMENT_SHADER);
        if (!shader)
            return 0;
        glAttachShader(program, shader);
    }

    glLinkProgram(program);
    GLint link_ok = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
    if (link_ok == GL_FALSE) {
        log_error("failed linking shader program");
        print_shader_log(program);
        glDeleteProgram(program);
        exit(1);
        return 0;
    }

    return program;
}

GLint shader_get_attrib(GLuint program, const char *name) {
    return glGetAttribLocation(program, name);
}

GLint shader_get_uniform(GLuint program, const char *name) {
    return glGetUniformLocation(program, name);
}
