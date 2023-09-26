#include "text_renderer.h"

#include <assert.h>

void text_renderer_init(text_renderer_t* renderer, path_t font) {
    renderer->shaderProgram = shader_program_create("src/glsl/text.v.glsl", 
                                                    "src/glsl/text.f.glsl");
    renderer->attributeCoord = shader_get_attrib(renderer->shaderProgram, "coord");
    renderer->uniformTex = shader_get_uniform(renderer->shaderProgram, "tex");
    renderer->uniformColor = shader_get_uniform(renderer->shaderProgram, "color");

    if (renderer->attributeCoord == -1 || renderer->uniformTex == -1 || renderer->uniformColor == -1) {
        log_error("failed initializing");
        return;
    }

    glGenBuffers(1, &renderer->vbo);

    glyph_cache_init(&renderer->gcache,
                     font,
                     0,
                     24,
                     true);

    glyph_gpu_atlas_init(&renderer->gatlas,
                         0,
                         renderer->gcache.table_fullness,
                         renderer->gcache.table_keys,
                         renderer->gcache.table_data);
}

void text_renderer_cleanup(text_renderer_t* renderer) {

}

void text_render_line(text_renderer_t* renderer,
                      byte_t* str,
                      size_t x, size_t y,
                      size_t pixel_size) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(renderer->shaderProgram);
    glUniform4f(renderer->uniformColor, 0, 0, 0, 1);

    glyph_gpu_atlas_activate(&renderer->gatlas, renderer->uniformTex);

    glEnableVertexAttribArray(renderer->attributeCoord);
    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
    glVertexAttribPointer(renderer->attributeCoord, 4, GL_FLOAT, GL_FALSE, 0, 0);


    float coords[6 * strlen((char*)str)][4];

    const byte_t* p;
    for (p = str; *p; p++) {
        FT_Glyph glyph;
        // TODO(ayham): ease this
        assert(hash_table_get(&renderer->gcache.table, (uint8_t*)p,
                            (hash_table_entry_t**)&glyph));
        FT_BitmapGlyph bglyph = (FT_BitmapGlyph) glyph; // maybe just store bitmaps glyphs in the cache?

        glyph_info* tex_info;
        // TODO(ayham): ease this
        assert(hash_table_get(&renderer->gatlas.table, (uint8_t*)p, 
                              (hash_table_entry_t**)&tex_info));
    }
}
