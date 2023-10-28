#include "text_renderer.h"

#include "texture_lender.h"

#include <assert.h>

#include <unicode/uloc.h>
#include <unicode/ubrk.h>
#include <unicode/ustring.h>

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

void text_renderer_undo(text_render_config* const conf) {
    free(conf->utf16_str);

    free(conf->wrap_indices_dat);
    conf->wrap_indices_cnt = 0;
}

void text_renderer_do(text_render_config* const conf) {
    conf->curr_x = conf->origin_x;
    conf->curr_y = conf->origin_y;

    if (conf->str_sz == 0) {
        conf->str_sz = strlen((char*)conf->str);
    }

    UErrorCode u_error_code = U_ZERO_ERROR;

    if (conf->utf16_str == NULL) {
        // ICU UBiDi requires UTF-16 strings
        u_strFromUTF8(NULL, 0, (int32_t*) &conf->utf16_sz,
                      (char*) conf->str, (int32_t) conf->str_sz,
                      &u_error_code);
        u_error_code = U_ZERO_ERROR;

        conf->utf16_str = malloc(sizeof(UChar) * (conf->utf16_sz));
        int32_t utf16_written = 0;
        u_strFromUTF8(conf->utf16_str, conf->utf16_sz, &utf16_written,
                      (char*) conf->str, (int32_t) conf->str_sz,
                      &u_error_code);
        if (U_FAILURE(u_error_code)) {
            log_error("u_strFromUTF8 failed, error_code: %i",
                      u_error_code);
            return;
        }
        assert(conf->utf16_sz == utf16_written);
    }

    UBiDi* bidi = ubidi_openSized(0, 0, &u_error_code);

    if (bidi == NULL) {
        log_error("ubidi_openSized failed");
        return;
    }

    ubidi_setPara(bidi, (UChar*) conf->utf16_str, conf->utf16_sz, 
                  conf->base_direction, NULL,
                  &u_error_code);

    if (U_FAILURE(u_error_code)) {
        log_error("failed ubidi_setPara");
        return;
    }

    //UBiDiLevel level = ubidi_getParaLevel(bidi);
    int32_t width_chars = __text_renderer_get_text_width(0, conf->utf16_sz);

    if (width_chars <= conf->max_line_width_chars) {
        __text_renderer_line(bidi, conf, 0, &u_error_code);
    } else {
        UBiDi* line = ubidi_openSized(conf->str_sz, 0, &u_error_code);
        if (line == NULL) {
            log_error("ubidi_openSized failed");
            return;
        }
        __text_renderer_calculate_line_wraps(conf);
        log_info("calced");

        int32_t start = 0, end = 0, line_number = 0;
        while (end < conf->utf16_sz) {
            // get logical end of line to render
            __text_renderer_get_line_break(conf, line_number++, &end);

            ubidi_setLine(bidi, start, end, line, &u_error_code);
            if (U_FAILURE(u_error_code)) {
                log_error("failed ubidi_setLine for line wrapping, error_code: %i",
                          u_error_code);
                log_debug("start: %i", start);
                log_debug("end: %i", end);
                log_debug("length: %i", end - start);
                break;
            }
            __text_renderer_line(line, conf, start, &u_error_code);
            __text_renderer_new_line(conf);

            start = end;
        }
        ubidi_close(line);
    }

    ubidi_close(bidi);
}

void __text_renderer_line(UBiDi* line, text_render_config* const conf,
                          int32_t logical_line_offset,
                          UErrorCode* error_code) {
    if (U_FAILURE(*error_code)) {
        log_error("__text_renderer_line passed-in failure error_code: %i",
                  *error_code);
        return;
    }

    if (U_FAILURE(*error_code)) {
        log_error("failed to run ubidi_getDirection, error_code: %i",
                  *error_code);
        return;
    }
    size_t count_runs = ubidi_countRuns(line, error_code);
    int32_t logical_start = 0, length = 0;
    for (size_t i = 0; i < count_runs; i++) {
        (void)ubidi_getVisualRun(line, i, &logical_start, &length);
        __text_renderer_run(conf, logical_start + logical_line_offset, length);
    }
}

void __text_renderer_run(text_render_config* const conf, 
                         int32_t logical_start, int32_t logical_length) {
    hb_buffer_reset(conf->renderer->hb_buf);
    hb_buffer_clear_contents(conf->renderer->hb_buf);
    hb_buffer_add_utf16(conf->renderer->hb_buf,
                       (uint16_t*) conf->utf16_str + logical_start, logical_length,
                       0, -1);

    hb_buffer_guess_segment_properties(conf->renderer->hb_buf);
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

void __text_renderer_calculate_line_wraps(text_render_config* const conf) {
    // TODO(ayham): requires character length and not UTF16 codepoint length

    if (conf->wrap_indices_dat && conf->wrap_indices_cnt)
        return; // already computed

    if (conf->max_line_width_chars >= conf->utf16_sz) { 
    no_wrap:
        conf->wrap_indices_cnt = 0;
        conf->wrap_indices_dat = NULL;
        return;
    }

    UErrorCode u_error = U_ZERO_ERROR;
    UBreakIterator* it = ubrk_open(UBRK_LINE, uloc_getDefault(),
                                   conf->utf16_str, conf->utf16_sz, &u_error);

    if (U_FAILURE(u_error)) {
        log_error("failed ubrk_open, error_code: %i", u_error);
        // TODO(ayham): check if line needs forceful breaking
        goto no_wrap;
    }

    int32_t logical_start = 0, logical_end = 0;
    if (ubrk_countAvailable() == 0) {
        log_warn("called get_line_break() with no reported available line breaks");
        conf->wrap_indices_cnt = (conf->utf16_sz / conf->max_line_width_chars) + 1;
        conf->wrap_indices_dat = calloc(conf->wrap_indices_cnt, sizeof(int32_t));

        for (int32_t i = 0; i < conf->wrap_indices_cnt; i++) {
            logical_end = logical_start + conf->max_line_width_chars;
            conf->wrap_indices_dat[i] = logical_end;
            logical_start = logical_end;
        }
    }

    // TODO(ayham): check if ubrk_first() returns after conf->max_line_width_chars

    bool mem_run = true;
    int32_t line = 0;
run_start:
    ubrk_first(it);
    line = 0;
    logical_start = 0;
    logical_end = 0;
    while (logical_end != UBRK_DONE && logical_end < conf->utf16_sz) {
        if (ubrk_isBoundary(it, logical_start + conf->max_line_width_chars)) {
            logical_end = ubrk_current(it);
        } else {
            logical_end = ubrk_following(it, logical_start + conf->max_line_width_chars);
            if (logical_end - (logical_start + 1) > conf->max_line_width_chars) {
                (void)ubrk_first(it);
                logical_end = ubrk_preceding(it,  logical_start + conf->max_line_width_chars);
            }
        }

        if (logical_end == logical_start) {
            if (logical_end + conf->max_line_width_chars >= conf->utf16_sz)
                logical_end = conf->utf16_sz;
            logical_end += conf->max_line_width_chars;
        }

        // insert pair into vector
        // TODO(ayham): implement vector and pair types
        logical_start = logical_end;
        if (!mem_run)
            conf->wrap_indices_dat[line] = logical_end;
        line++;
    }
    if (!mem_run) return;

    conf->wrap_indices_cnt = line + 1;
    conf->wrap_indices_dat = calloc(conf->wrap_indices_cnt, sizeof(int32_t));
    mem_run = false;
    goto run_start;
}

void __text_renderer_get_line_break(text_render_config* const conf,
                                    int32_t line_number, 
                                    int32_t* out_logical_end) {
    if (conf->wrap_indices_cnt == 0) {
        *out_logical_end = conf->utf16_sz;
        return;
    }
    *out_logical_end = conf->wrap_indices_dat[line_number];
}

size_t __text_renderer_get_text_width(int32_t logical_start,
                                      int32_t logical_end) {
    // TODO(ayham): may cause visual errors, needs research and confirmation:
    // Since logcial_* refer to UTF16 format, where every character is represented
    // by a single UChar, this *should* be fine.
    return logical_end - logical_start;
}

void __text_renderer_new_line(text_render_config* const conf) {
    conf->curr_y -= conf->renderer->gcache.ft_face->height >> 6;
    conf->curr_x = conf->origin_x;
}

void text_renderer_update_window_size(text_renderer_t* renderer, int width, int height) {
    renderer->scr_width = width;
    renderer->scr_height = height;

    glm_ortho(0.0f, (float) width, 0.0f, (float) height, -1, 1, renderer->projection);
}
