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
    conf->utf16_str = NULL;

    free(conf->wrap_runs_dat);
    conf->wrap_runs_cnt = 0;
    conf->wrap_runs_dat = NULL;

    free(conf->it_char);
    conf->it_char = NULL;
    conf->char_num = 0;
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
    __text_renderer_calculate_line_char_width(conf);
    __text_renderer_calculate_line_wraps(conf);

    if (conf->char_num <= conf->max_line_width_chars) {
        __text_renderer_line(bidi, conf, 0, &u_error_code);
    } else {
        UBiDi* line = ubidi_openSized(conf->str_sz, 0, &u_error_code);
        if (line == NULL) {
            log_error("ubidi_openSized failed");
            return;
        }

        int32_t start = 0, end = 0, line_number = 0;
        while (line_number < conf->wrap_runs_cnt) {
            // get logical end of line to render
            __text_renderer_get_line_break(conf, line_number, &start, &end);

            ubidi_setLine(bidi, start, end, line, &u_error_code);
            if (U_FAILURE(u_error_code)) {
                log_error("failed ubidi_setLine, error_code: %i",
                          u_error_code);
                log_debug("start: %i", start);
                log_debug("end: %i", end);
                log_debug("length: %i", end - start);
                log_debug("line number: %i", line_number);
                break;
            }
            __text_renderer_line(line, conf, start, &u_error_code);
            __text_renderer_new_line(conf);

            start = end;
            line_number++;
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

    if (conf->wrap_runs_dat && conf->wrap_runs_cnt)
        return; // already computed

    if (conf->max_line_width_chars >= conf->char_num) { 
        conf->wrap_runs_cnt = 0;
        conf->wrap_runs_dat = NULL;
        return;
    }

    UErrorCode u_error = U_ZERO_ERROR;
    UBreakIterator* it_line = ubrk_open(UBRK_LINE, NULL,
                                   conf->utf16_str, conf->utf16_sz - 1, &u_error);

    if (U_FAILURE(u_error)) {
        log_error("failed ubrk_open for line wrap, error_code: %i", u_error);
    }

    ubrk_first(it_line);
    int32_t logical_start = 0, logical_end = 0;

    // ubidi for counting conf->max_line_width_chars as visual chars
    UBiDi* bidi = ubidi_openSized(0, 0, &u_error);

    if (bidi == NULL) {
        log_error("ubidi_openSized failed");
        goto calc_wrap_bidi_end;
    }

    ubidi_setPara(bidi, (UChar*) conf->utf16_str, conf->utf16_sz, 
                  conf->base_direction, NULL,
                  &u_error);

    if (U_FAILURE(u_error)) {
        log_error("failed ubidi_setPara");
        goto calc_wrap_bidi_end;
    }

    UBiDi* bidi_line = ubidi_openSized(conf->str_sz, 0, &u_error);
    if (bidi_line == NULL) {
        log_error("ubidi_openSized failed");
        goto calc_wrap_bidi_end;
    }
    ubidi_setLine(bidi, 0, conf->utf16_sz, bidi_line, &u_error);
    if (U_FAILURE(u_error)) {
        log_error("failed ubidi_setLine, error_code: %i", u_error);
        goto calc_wrap_bidi_end;
    }

    bool mem_run = true;
    int32_t bidi_result_length = ubidi_getResultLength(bidi_line);
    int32_t* vis2log_map = calloc(bidi_result_length, sizeof(int32_t));
    int32_t* log2vis_map = calloc(bidi_result_length, sizeof(int32_t));

    if (!vis2log_map || !log2vis_map) {
        log_error("wow no more memory, sad");
        exit(1);
    }

    ubidi_getVisualMap(bidi_line, 
                        vis2log_map,
                        &u_error);
    if (U_FAILURE(u_error)) {
        log_var(vis2log_map);
        log_error("failed ubidi_getVisualMap");
        log_var(u_error);
        goto calc_wrap_end;
    }
    ubidi_getLogicalMap(bidi_line, 
                        log2vis_map,
                        &u_error);
    if (U_FAILURE(u_error)) {
        log_var(log2vis_map);
        log_error("failed ubidi_getIndexMap");
        log_var(u_error);
        goto calc_wrap_end;
    }

    int32_t lines = 0;
    int32_t length = ubidi_getLength(bidi_line);

    bool is_it_line_empty = ubrk_first(it_line) == 0
                            && ubrk_next(it_line) == conf->utf16_sz - 1;

calc_wrap_run_start:
    lines = 0;
    logical_start = 0;
    logical_end = 0;
    bool reached_end = false;
    while (!reached_end) {
        ubrk_first(it_line);
        int32_t final_logical_start = logical_start;
        int32_t final_logical_end = logical_end;

        int32_t margin_visual_end = log2vis_map[logical_start] + conf->max_line_width_chars;
        int32_t margin_logical_end = vis2log_map[margin_visual_end];

        if (length <= margin_visual_end) {
            final_logical_end = conf->utf16_sz;
            reached_end = true;
        } else if (is_it_line_empty) {
            final_logical_end = margin_logical_end;
        } else {
            UChar c = conf->utf16_str[margin_logical_end];
            int32_t cand_logical_end = margin_logical_end;
            int32_t cand_visual_end = margin_visual_end;

            // skip whitespace
            while (cand_visual_end < length 
                && (u_isspace(c) 
                || u_charType(c) == U_CONTROL_CHAR
                || u_charType(c) == U_NON_SPACING_MARK)) {
                cand_visual_end++;
                cand_logical_end = vis2log_map[cand_visual_end];
                c = conf->utf16_str[cand_logical_end];
            }
            cand_visual_end++;
            cand_logical_end = ubrk_preceding(it_line, cand_visual_end);

            if (logical_start >= cand_logical_end)
                final_logical_end = margin_logical_end;
            else
                final_logical_end = cand_logical_end;
        }

        // ensure final_logical_end is not way too far back
        //
        if (log2vis_map[final_logical_end] - log2vis_map[final_logical_start] < 
            (int32_t)(0.75f * (float) conf->max_line_width_chars) 
            && !is_it_line_empty && !reached_end)
            final_logical_end = margin_logical_end;

        if (!mem_run) {
            conf->wrap_runs_dat[lines].start = final_logical_start;
            conf->wrap_runs_dat[lines].end = final_logical_end;
        }
        logical_start = final_logical_end;
        logical_end = final_logical_end;
        lines++;
    }
    if (!mem_run) goto calc_wrap_end;

    conf->wrap_runs_cnt = lines;
    conf->wrap_runs_dat = calloc(conf->wrap_runs_cnt, sizeof(wrap_run_indices_t));
    mem_run = false;
    goto calc_wrap_run_start;

calc_wrap_end:
    free(vis2log_map);
    free(log2vis_map);
calc_wrap_bidi_end:
    ubrk_close(it_line);
    return;
}

void __text_renderer_calculate_line_char_width(text_render_config* const conf) {
    if (conf->it_char) return;
    UErrorCode u_error = U_ZERO_ERROR;

    conf->it_char = ubrk_open(UBRK_CHARACTER, uloc_getDefault(), 
                              conf->utf16_str, conf->utf16_sz, &u_error);

    if (U_FAILURE(u_error)) {
        log_error("failed ubrk_open for line width, error_code: %i", u_error);
        // goto no_wrap; // just continue and pray it doesn't break, 
        // can't think of a situation where this may be an issue,
        // except maybe malloc problems, and i am not going to handle that :D
        // hopefully won't come back to bite. goodluck!
    }

    conf->char_num = 0;
    int32_t logical = 0;
    while (logical != UBRK_DONE){
            logical = ubrk_next(conf->it_char);
            conf->char_num++;
    }
    (void)ubrk_first(conf->it_char);
}

void __text_renderer_get_line_break(text_render_config* const conf,
                                    int32_t line_number, 
                                    int32_t* out_logical_start,
                                    int32_t* out_logical_end) {
    if (conf->wrap_runs_cnt == 0) {
        *out_logical_start = 0;
        *out_logical_end = conf->utf16_sz;
        return;
    }
    *out_logical_start = conf->wrap_runs_dat[line_number].start;
    *out_logical_end = conf->wrap_runs_dat[line_number].end;
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
