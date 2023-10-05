/*
 * Only job is to keep track of taken texture slots on the gpu.
 * DOES NOT handle cleaning.
 *
 * Returns a texture ID from glGenTexture
 * */
#ifndef TEXTURE_LENDER_H
#define TEXTURE_LENDER_H

#include <GL/glew.h>

#include <stdbool.h>

typedef struct {
    GLsizei max_textures;
    GLsizei available_textures;

    bool *textureIDs;
} texture_lender;

void texture_lender_init(GLsizei max_textures);

GLuint texture_lender_request();
void texture_lender_return(GLuint texID);

extern texture_lender __t_lender;

#endif
