#include "text_renderer.h"

#include "texture_lender.h"

#include <assert.h>

void text_renderer_init(text_renderer_t* renderer, path_t font,
                        size_t width, size_t height, size_t font_pixel_size) {
    renderer->shaderProgram = shader_program_create("glsl/text.v.glsl", 
                                                    "glsl/text.f.glsl");
    renderer->attributeCoord = shader_get_attrib(renderer->shaderProgram, "coord");
    renderer->uniformTex = shader_get_uniform(renderer->shaderProgram, "tex");
    renderer->uniformColor = shader_get_uniform(renderer->shaderProgram, "textColor");
    renderer->uniformProjection = shader_get_uniform(renderer->shaderProgram, "projection");

    glGenVertexArrays(1, &renderer->vao);
    glGenBuffers(1, &renderer->vbo);
    glBindVertexArray(renderer->vao);
    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glGenBuffers(1, &renderer->ibo);

    glyph_cache_init(&renderer->gcache, font, 512, font_pixel_size, true);

    renderer->scr_width = width;
    renderer->scr_height = height;
    renderer->font_pixel_size = font_pixel_size;

    glm_ortho(0.0f, (float) width, 0.0f, (float) height, -1, 1, renderer->projection);

    hb_blob_t* blob = hb_blob_create_from_file((char*) font.fullPath.bytes);
    renderer->hb_face = hb_face_create(blob, 0);
    renderer->hb_font = hb_ft_font_create(renderer->gcache.ft_face, NULL);
    renderer->hb_buf =  hb_buffer_create();
    assert(hb_buffer_allocation_successful(renderer->hb_buf));

    hb_blob_destroy(blob);

    log_info("created text renderer");
}

void text_renderer_cleanup(text_renderer_t* renderer) {
    hb_font_destroy(renderer->hb_font);
    hb_face_destroy(renderer->hb_face);
    hb_buffer_destroy(renderer->hb_buf);

    glDeleteBuffers(1, &renderer->vbo);
    glDeleteBuffers(1, &renderer->ibo);
}

void text_renderer_line(text_renderer_t* renderer,
                      byte_t* str,
                      GLfloat origin_x, GLfloat origin_y) {
    hb_buffer_clear_contents(renderer->hb_buf);
    hb_buffer_add_utf8(renderer->hb_buf, (char*) str, -1, 0, -1);

    hb_buffer_guess_segment_properties(renderer->hb_buf);

    hb_shape(renderer->hb_font, renderer->hb_buf, NULL, 0);

    unsigned int hb_glyph_count;
    hb_glyph_info_t* hb_glyph_info = hb_buffer_get_glyph_infos(renderer->hb_buf, &hb_glyph_count);
    hb_glyph_position_t* hb_glyph_pos = hb_buffer_get_glyph_positions(renderer->hb_buf, &hb_glyph_count);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glUseProgram(renderer->shaderProgram);
    glUniform3f(renderer->uniformColor, 0, 1, 0);
    glUniformMatrix4fv(renderer->uniformProjection, 1, GL_FALSE, (GLfloat*) renderer->projection);

    typedef struct {
        GLfloat x;
        GLfloat y;
        GLfloat s;
        GLfloat t;
    } point;

    point coords[6 * hb_glyph_count];
    size_t n = 0;

    for (unsigned int i = 0; i < hb_glyph_count; i++) {
        hb_glyph_info_t hb_info = hb_glyph_info[i];
        hb_glyph_position_t hb_pos = hb_glyph_pos[i];

        glyph_info* info = NULL;
        info = glyph_cache_retrieve(&renderer->gcache, hb_info.codepoint);
        assert(info != NULL);

        // NOTE: left-bottom origin
        int32_t awidth = renderer->gcache.awidth;
        int32_t aheight = renderer->gcache.aheight;
        int32_t w = info->bglyph->bitmap.width;
        int32_t h = info->bglyph->bitmap.rows;

        float ratio_w = (float) w / awidth;
        float ratio_h = (float) h / aheight;

        // char quad ccw
        GLfloat x0 = origin_x + info->bearing_x;
        GLfloat y0 = origin_y + info->bearing_y;
        GLfloat s0 = info->texture_x;
        GLfloat t0 = info->texture_y;

        GLfloat x1 = x0;
        GLfloat y1 = y0 - h;
        GLfloat s1 = s0;
        GLfloat t1 = t0 + ratio_h;
        
        GLfloat x2 = x0 + w;
        GLfloat y2 = y1;
        GLfloat s2 = s1 + ratio_w;
        GLfloat t2 = t1;
        
        GLfloat x3 = x2;
        GLfloat y3 = y0;
        GLfloat s3 = s2;
        GLfloat t3 = t0;

        coords[n++] = (point) {x0, y0, s0, t0};
        coords[n++] = (point) {x1, y1, s1, t1};
        coords[n++] = (point) {x2, y2, s2, t2};

        coords[n++] = (point) {x2, y2, s2, t2};
        coords[n++] = (point) {x3, y3, s3, t3};
        coords[n++] = (point) {x0, y0, s0, t0};

        origin_x += hb_pos.x_advance >> 6;
        origin_y += hb_pos.y_advance >> 6;
    }

    glBindVertexArray(renderer->vao);
    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->ibo);

    glActiveTexture(renderer->gcache.atexID);
    glBindTexture(GL_TEXTURE_2D, renderer->gcache.atexOBJ);
    glBufferData(GL_ARRAY_BUFFER, sizeof(coords), coords, GL_STATIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, n);
}

void text_renderer_update_window_size(text_renderer_t* renderer, int width, int height) {
    renderer->scr_width = width;
    renderer->scr_height = height;

    glm_ortho(0.0f, (float) width, 0.0f, (float) height, -1, 1, renderer->projection);
}
