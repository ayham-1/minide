#ifndef SHADERS_UTIL_H
#define SHADERS_UTIL_H

#include <GL/glew.h>

GLuint shader_create(const char *filename, GLenum type);
GLuint shader_program_create(const char *vertexfile, const char *fragmentfile);
GLint shader_get_attrib(GLuint program, const char *name);
GLint shader_get_uniform(GLuint program, const char *name);

#endif
