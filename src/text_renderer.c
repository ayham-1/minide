#include "text_renderer.h"

#include "texture_lender.h"

#include <assert.h>

#include <unicode/ubrk.h>

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

    glyph_cache_init(&renderer->gcache, font, 512, font_pixel_size);

    renderer->scr_width = width;
    renderer->scr_height = height;
    renderer->font_pixel_size = font_pixel_size;

    glm_ortho(0.0f, (float) width, 0.0f, (float) height, -1, 1, renderer->projection);

    hb_blob_t* blob = hb_blob_create_from_file((char*) font.fullPath.bytes);
    assert(blob);
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

void text_renderer_do(text_render_config* const conf) {
    conf->curr_x = conf->origin_x;
    conf->curr_y = conf->origin_y;

    size_t num_bytes = conf->num_bytes; 

    UErrorCode u_error_code = U_ZERO_ERROR;
    UBiDi* bidi = ubidi_openSized(num_bytes, 0, &u_error_code);

    if (bidi == NULL) {
        log_error("ubidi_openSized failed");
        return;
    }

    ubidi_setPara(bidi, (UChar*) conf->str, num_bytes, 
                  conf->base_direction, NULL,
                  &u_error_code);

    if (U_SUCCESS(u_error_code)) {
        UBiDiLevel level = 1 & ubidi_getParaLevel(bidi);
        size_t width_chars = __text_renderer_get_text_width(conf->str, 0, num_bytes);

        if (width_chars <= conf->max_line_width_chars) {
            // everything fits into one line
            // consider length of width for RTL or LTR
            // render_line
            __text_renderer_line(bidi, conf, 0, num_bytes, &u_error_code);
        } else {
            UBiDi* line = ubidi_openSized(num_bytes, 0, &u_error_code);
            if (line != NULL) {
                int32_t start = 0, end = 0;
                for (;;) {
                    // get logical end of line to render
                    __text_renderer_get_line_break(bidi, conf, 
                                                   start, &end);

                    ubidi_setLine(bidi, start, end, line, &u_error_code);

                    if (U_SUCCESS(u_error_code)) {
                        // consider length of width for RTL or LTR
                        // render_line
                        __text_renderer_line(line, conf, start, end, &u_error_code);
                    }

                    if (end >= num_bytes) break;
                    start = end;
                }
            }
            ubidi_close(line);
        }
    }

    ubidi_close(bidi);
}

void __text_renderer_line(UBiDi* line, text_render_config* const conf,
                          int32_t start, int32_t end,
                          UErrorCode* error_code) {
    UBiDiDirection direction = ubidi_getDirection(line);
    log_debug("direction: %i", direction);
    if (direction != UBIDI_MIXED) {
        size_t count_runs = ubidi_countRuns(line, error_code);
        log_debug("count_runs: %i", count_runs);
        __text_renderer_run(conf, start, end, direction);
    } else {
        size_t count_runs = ubidi_countRuns(line, error_code);
        log_debug("count_runs: %i", count_runs);
        int32_t length = 0;
        for (size_t i = 0; i < count_runs; i++) {
            direction = ubidi_getVisualRun(line, i, &start, &length);
            __text_renderer_run(conf, start, start + length, direction);
        }
    }
}

void __text_renderer_run(text_render_config* const conf, 
                         int32_t logical_start, int32_t logical_limit,
                         UBiDiDirection direction) {
    hb_buffer_clear_contents(conf->renderer->hb_buf);
    hb_buffer_add_utf8(conf->renderer->hb_buf, (char*) conf->str, -1, 0, -1);

    hb_buffer_guess_segment_properties(conf->renderer->hb_buf);

    //hb_buffer_set_direction(conf->renderer->hb_buf, direction == UBIDI_LTR ? HB_DIRECTION_LTR : HB_DIRECTION_RTL);
    //hb_buffer_set_script(renderer->hb_buf, HB_SCRIPT_LATIN);
    //hb_buffer_set_language(renderer->hb_buf, hb_language_from_string("en", -1));

    hb_shape(conf->renderer->hb_font, conf->renderer->hb_buf, NULL, 0);

    unsigned int hb_glyph_count;
    hb_glyph_info_t* hb_glyph_info = hb_buffer_get_glyph_infos(conf->renderer->hb_buf, 
                                                               &hb_glyph_count);
    hb_glyph_position_t* hb_glyph_pos = hb_buffer_get_glyph_positions(conf->renderer->hb_buf, 
                                                                      &hb_glyph_count);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glUseProgram(conf->renderer->shaderProgram);
    glUniform3f(conf->renderer->uniformColor, 0, 1, 0);
    glUniformMatrix4fv(conf->renderer->uniformProjection, 1, GL_FALSE, (GLfloat*) conf->renderer->projection);

    typedef struct {
        GLfloat x;
        GLfloat y;
        GLfloat s;
        GLfloat t;
    } point;

    point coords[6 * hb_glyph_count];
    size_t n = 0;

    //if (direction == UBIDI_RTL) i = hb_glyph_count - 1;

    for (unsigned int i = 0; i < hb_glyph_count; i++) {
        hb_glyph_info_t hb_info = hb_glyph_info[i];
        hb_glyph_position_t hb_pos = hb_glyph_pos[i];

        glyph_info* info = NULL;
        info = glyph_cache_retrieve(&conf->renderer->gcache, hb_info.codepoint);
        assert(info != NULL);

        // NOTE: left-bottom origin
        uint32_t awidth = conf->renderer->gcache.awidth;
        uint32_t aheight = conf->renderer->gcache.aheight;
        uint32_t w = info->bglyph->bitmap.width;
        uint32_t h = info->bglyph->bitmap.rows;

        float ratio_w = (float) w / awidth;
        float ratio_h = (float) h / aheight;

        // char quad ccw
        GLfloat x0 = conf->curr_x + info->bearing_x;
        GLfloat y0 = conf->curr_y + info->bearing_y;
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

        conf->curr_x += hb_pos.x_advance >> 6;
        conf->curr_y += hb_pos.y_advance >> 6;
    }

    glBindVertexArray(conf->renderer->vao);
    glBindBuffer(GL_ARRAY_BUFFER, conf->renderer->vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, conf->renderer->ibo);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, conf->renderer->gcache.atexOBJ);
    glBufferData(GL_ARRAY_BUFFER, sizeof(coords), coords, GL_STATIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, n);
}

size_t __text_renderer_get_text_width(const byte_t* const str, 
                                      int32_t logical_start,
                                      int32_t logical_end) {
    size_t char_width = 0;

    UBreakIterator* it;
    UErrorCode err = U_ZERO_ERROR;
    it = ubrk_open(UBRK_CHARACTER, 0, 
                   (UChar*) str, strlen((char*) str), &err);
    if (U_FAILURE(err)) {
        log_error("unable to count character width of text");
        return char_width;
    }

    int32_t p = ubrk_first(it);
    while (p != UBRK_DONE) {
        char_width++;
        p = ubrk_next(it);
    }

    ubrk_close(it);
    return char_width;
}

void __text_renderer_get_line_break(UBiDi* bidi,
                                    text_render_config* const conf,
                                    int32_t logical_line_start, 
                                    int32_t* out_logical_end) {
    //int32_t logical_run_end = 0;
    //ubidi_getLogicalRun(bidi, logical_line_start, &logical_run_end, NULL);

    //size_t count_run_chars = __text_renderer_get_text_width(conf->str,
    //                                                        logical_line_start,
    //                                                        logical_run_end);

    // TODO(ayham): implement actual line breaking algorithm
    out_logical_end += conf->max_line_width_chars;
}

void text_renderer_update_window_size(text_renderer_t* renderer, int width, int height) {
    renderer->scr_width = width;
    renderer->scr_height = height;

    glm_ortho(0.0f, (float) width, 0.0f, (float) height, -1, 1, renderer->projection);
}
