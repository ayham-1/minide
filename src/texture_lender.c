#include "texture_lender.h"

#include <malloc.h>
#include <assert.h>
#include <string.h>

texture_lender __t_lender;

void texture_lender_init(GLsizei max_textures) {
    __t_lender.max_textures = max_textures;
    __t_lender.available_textures = max_textures;

    __t_lender.textureIDs = malloc(sizeof(bool) * max_textures);
    memset(__t_lender.textureIDs, false, sizeof(bool) * max_textures);
}

GLuint texture_lender_request() {
    assert(__t_lender.max_textures >= __t_lender.available_textures);

    for (GLsizei id = 0; id <= __t_lender.max_textures; id++) {
        if (__t_lender.textureIDs[id] == false) {
            __t_lender.available_textures--;
            __t_lender.textureIDs[id] = true;
            return id;
        }
    }

    return 0; /* uh oh stinky!!1 */
}

void texture_lender_return(GLuint texID) {
    if (!__t_lender.textureIDs[texID]) return;

    __t_lender.textureIDs[texID] = false;
    __t_lender.available_textures++;
}
