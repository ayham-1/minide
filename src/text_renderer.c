#include "minide/text_renderer.h"

#include "minide/shaper.h"

#include "minide/texture_lender.h"

#include "minide/helpers/icu_checker.h"

#include <assert.h>

#include <unicode/ubrk.h>
#include <unicode/uloc.h>
#include <unicode/ustring.h>

void text_renderer_init(text_renderer_t * renderer, enum FontFamilyStyle font_style, size_t width, size_t height,
			size_t font_pixel_size)
{
	renderer->shaderProgram = shader_program_create("glsl/text.v.glsl", "glsl/text.f.glsl");
	renderer->attributeCoord = shader_get_attrib(renderer->shaderProgram, "coord");
	renderer->uniformTex = shader_get_uniform(renderer->shaderProgram, "tex");
	renderer->uniformColor = shader_get_uniform(renderer->shaderProgram, "textColor");
	renderer->uniformProjection = shader_get_uniform(renderer->shaderProgram, "projection");
	renderer->uniformSingleColor = shader_get_uniform(renderer->shaderProgram, "singleColor");

	glGenVertexArrays(1, &renderer->vao);
	glGenBuffers(1, &renderer->vbo);
	glBindVertexArray(renderer->vao);
	glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glGenBuffers(1, &renderer->ibo);

	renderer->scr_width = width;
	renderer->scr_height = height;
	renderer->font_pixel_size = font_pixel_size;

	glm_ortho(0.0f, (float)width, 0.0f, (float)height, -1, 1, renderer->projection);

	renderer->font_style = font_style;

	log_info("created text renderer");
}

void text_renderer_cleanup(text_renderer_t * renderer)
{
	glDeleteBuffers(1, &renderer->vbo);
	glDeleteBuffers(1, &renderer->ibo);
}

void text_renderer_undo(text_render_config * const conf)
{
	free(conf->utf16_str);
	conf->utf16_str = NULL;

	free(conf->lines);
	conf->lines = NULL;
	conf->lines_cnt = 0;

	free(conf->it_char);
	conf->it_char = NULL;
	conf->char_num = 0;
}

void text_renderer_do(text_render_config * const conf)
{
	assert(conf);

	conf->curr_x = conf->origin_x;
	conf->curr_y = conf->origin_y;

	assert(conf->utf8_str || conf->utf16_str);

	if (conf->utf8_str && conf->utf8_sz == 0) {
		conf->utf8_sz = strlen((char *)conf->utf8_str);
	}

	if (conf->utf16_str && conf->utf16_sz == 0) {
		conf->utf16_sz = u_strlen(conf->utf16_str);
	}

	UErrorCode u_error = U_ZERO_ERROR;

	if (conf->utf16_str == NULL) {
		// ICU UBiDi requires UTF-16 strings
		u_strFromUTF8(NULL, 0, (int32_t *)&conf->utf16_sz, (char *)conf->utf8_str, (int32_t)conf->utf8_sz,
			      &u_error);
		u_error = U_ZERO_ERROR;

		conf->utf16_str = malloc(sizeof(UChar) * (conf->utf16_sz));
		int32_t utf16_written = 0;

		ICU_CHECK_FAIL(u_strFromUTF8(conf->utf16_str, conf->utf16_sz, &utf16_written, (char *)conf->utf8_str,
					     (int32_t)conf->utf8_sz, &u_error),
			       clean_utf16_str);
		assert(conf->utf16_sz == utf16_written);
	}

	UBiDi * bidi = ICU_CHECK_FAIL(ubidi_openSized(0, 0, &u_error), clean_utf16_str);

	ICU_CHECK_FAIL(
	    ubidi_setPara(bidi, (UChar *)conf->utf16_str, conf->utf16_sz, conf->base_direction, NULL, &u_error),
	    clean_bidi);

	__text_renderer_calculate_lines(conf);

	int32_t start = 0, end = 0, line_number = 0;
	while (line_number < conf->lines_cnt) {
		// get logical end of line to render
		__text_renderer_get_line_break(conf, line_number, &start, &end);

		if (start == end)
			break;

		UBiDi * line = ICU_CHECK_FAIL(ubidi_openSized(0, 0, &u_error), clean_bidi);
		ICU_CHECK_FAIL(ubidi_setLine(bidi, start, end, line, &u_error), setLine_debug);
		if (U_FAILURE(u_error)) {
		setLine_debug:
			log_debug("start: %i", start);
			log_debug("end: %i", end);
			log_debug("length: %i", end - start);
			log_debug("line number: %i", line_number);
			log_debug("max lines: %i", conf->lines_cnt);
			log_var(conf->utf16_sz);
			goto clean_bidi;
		}
		__text_renderer_line(line, conf, start, &u_error);
		__text_renderer_new_line(conf);

		start = end;
		line_number++;
	}

clean_bidi:
	ubidi_close(bidi);

	return;

clean_utf16_str:
	free(conf->utf16_str);
	conf->utf16_sz = 0;
}

void __text_renderer_line(UBiDi * line, text_render_config * const conf, int32_t logical_line_offset,
			  UErrorCode * error_code)
{
	assert(U_SUCCESS(*error_code));

	size_t count_runs = ubidi_countRuns(line, error_code);
	int32_t logical_start = 0, length = 0;
	for (size_t i = 0; i < count_runs; i++) {
		(void)ubidi_getVisualRun(line, i, &logical_start, &length);
		__text_renderer_run(conf, logical_start + logical_line_offset, length);
	}
}

void __text_renderer_run(text_render_config * const conf, int32_t logical_start, int32_t logical_length)
{
	shaper_holder holder = (shaper_holder){
	    .do_font_fallback = true,
	    .do_emoji_fallback = true,
	    .pixel_size = conf->renderer->font_pixel_size,
	    .preferred_style = conf->renderer->font_style,

	    .utf16_str = conf->utf16_str + logical_start,
	    .logical_length = logical_length,
	};
	shaper_do(&holder);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glUseProgram(conf->renderer->shaderProgram);
	glUniform3f(conf->renderer->uniformColor, 0, 1, 0);
	glUniformMatrix4fv(conf->renderer->uniformProjection, 1, GL_FALSE, (GLfloat *)conf->renderer->projection);

	typedef struct {
		GLfloat x;
		GLfloat y;
		GLfloat s;
		GLfloat t;
	} point;

	for (size_t run_num = 0; run_num < holder.runs_fullness; run_num++) {
		shaper_font_run_t run = holder.runs[run_num];
		glUniform1i(conf->renderer->uniformSingleColor, !FT_HAS_COLOR(run.font->face));

		point coords[6 * run.glyph_count];
		size_t n = 0;

		glyph_cache * gcache = font_get_glyph_cache(run.font, conf->renderer->font_pixel_size);
		assert(gcache != NULL);

		for (unsigned int i = 0; i < run.glyph_count; i++) {
			hb_glyph_info_t hb_info = run.glyph_infos[i];
			hb_glyph_position_t hb_pos = run.glyph_pos[i];

			glyph_info * info =
			    font_get_glyph(run.font, hb_info.codepoint, conf->renderer->font_pixel_size);
			assert(info != NULL);

			// NOTE: left-bottom origin
			uint32_t awidth = gcache->awidth;
			uint32_t aheight = gcache->aheight;
			uint32_t w = info->bglyph->bitmap.width;
			uint32_t h = info->bglyph->bitmap.rows;

			float ratio_w = (float)w / awidth;
			float ratio_h = (float)h / aheight;

			if (run.scale != 1) {
				w = ((float)w) * run.scale;
				h = ((float)h) * run.scale;
			}

			// char quad ccw
			GLfloat x0 = conf->curr_x;
			GLfloat y0 = conf->curr_y;
			if (run.scale != 1) {
				x0 += info->bearing_x * run.scale;
				y0 += info->bearing_y * run.scale;
			} else {
				x0 += info->bearing_x;
				y0 += info->bearing_y;
			}
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

			coords[n++] = (point){x0, y0, s0, t0};
			coords[n++] = (point){x1, y1, s1, t1};
			coords[n++] = (point){x2, y2, s2, t2};

			coords[n++] = (point){x2, y2, s2, t2};
			coords[n++] = (point){x3, y3, s3, t3};
			coords[n++] = (point){x0, y0, s0, t0};

			if (run.scale != 1) {
				conf->curr_x += ((float)(hb_pos.x_advance >> 6)) * run.scale;
				conf->curr_y += ((float)(hb_pos.y_advance >> 6)) * run.scale;
			} else {
				conf->curr_x += hb_pos.x_advance >> 6;
				conf->curr_y += hb_pos.y_advance >> 6;
			}
		}

		glBindVertexArray(conf->renderer->vao);
		glBindBuffer(GL_ARRAY_BUFFER, conf->renderer->vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, conf->renderer->ibo);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gcache->atexOBJ);
		glBufferData(GL_ARRAY_BUFFER, sizeof(coords), coords, GL_DYNAMIC_DRAW);
		glDrawArrays(GL_TRIANGLES, 0, n);
	}

	shaper_undo(&holder);
}

void __text_renderer_calculate_lines(text_render_config * const conf)
{
	if (conf->lines && conf->lines_cnt)
		return; // already computed

	UErrorCode u_error = U_ZERO_ERROR;
	conf->para = ICU_CHECK_FAIL(ubidi_openSized(0, 0, &u_error), no_clean);

	ICU_CHECK_FAIL(ubidi_setPara(conf->para, conf->utf16_str, conf->utf16_sz, conf->base_direction, NULL, &u_error),
		       clean_para);

	bool mem_run = true;
	int32_t total_count = 0;
	int32_t current_line = 0;
line_calcs_start:
	for (int32_t line_start = 0; line_start < conf->utf16_sz;) {
		int32_t line_end = line_start;
		while (line_end < conf->utf16_sz && '\n' != conf->utf16_str[line_end++])
			;

		line_indices_t line;
		line.start = line_start;
		line.end = ('\n' == conf->utf16_str[line_end - 1]) ? line_end - 1 : line_end;
		int32_t cnt = 0;

		if (line.end - line.start != 0) {
			if (mem_run) {
				__text_renderer_calculate_soft_wraps(conf, line, &cnt, NULL);
			} else {
				__text_renderer_calculate_soft_wraps(conf, line, &cnt, &conf->lines[current_line]);
				current_line += cnt;
			}
		}

		// if (0 == cnt) {
		//	log_error("something bad happened");
		//	exit(1);
		// }
		total_count += cnt;

		line_start = line_end;
	}

	if (mem_run) {
		conf->lines_cnt = total_count;
		conf->lines = calloc(conf->lines_cnt, sizeof(line_indices_t));

		mem_run = false;
		goto line_calcs_start;
	}

clean_para:
	ubidi_close(conf->para);

no_clean:;
}

void __text_renderer_calculate_soft_wraps(text_render_config * const conf, line_indices_t line, int32_t * cnt,
					  line_indices_t * arr_start)
{
	assert(cnt);
	int32_t sz = line.end - line.start;

	UErrorCode u_error = U_ZERO_ERROR;
	UBreakIterator * it_line =
	    ICU_CHECK_FAIL(ubrk_open(UBRK_LINE, NULL, conf->utf16_str + line.start, sz, &u_error), no_clean);

	ubrk_first(it_line);

	UBiDi * bidi_line = ICU_CHECK_FAIL(ubidi_openSized(0, 0, &u_error), clean_it_bidi);
	ICU_CHECK_FAIL(ubidi_setLine(conf->para, line.start, line.end, bidi_line, &u_error), clean_wrap_bidi);

	bool mem_run = (arr_start == NULL);
	int32_t bidi_result_length = ubidi_getResultLength(bidi_line);
	int32_t * vis2log_map = calloc(bidi_result_length, sizeof(int32_t));
	int32_t * log2vis_map = calloc(bidi_result_length, sizeof(int32_t));

	if (!vis2log_map || !log2vis_map) {
		log_error("wow no more memory, sad");
		exit(1);
	}

	ICU_CHECK_FAIL(ubidi_getVisualMap(bidi_line, vis2log_map, &u_error), clean_wrap_mem);
	ICU_CHECK_FAIL(ubidi_getLogicalMap(bidi_line, log2vis_map, &u_error), clean_wrap_mem);

	int32_t lines = 0;
	int32_t length = ubidi_getLength(bidi_line);

	bool is_it_line_empty = ubrk_first(it_line) == 0 && ubrk_next(it_line) == (sz - 1);

	lines = 0;
	int32_t logical_start = 0;
	int32_t logical_end = 0;
	bool reached_end = false;
	while (!reached_end) {
		ubrk_first(it_line);
		int32_t final_logical_start = logical_start;
		int32_t final_logical_end = logical_end;

		int32_t margin_visual_end =
		    log2vis_map[logical_start] + conf->max_line_width_chars; // todo: cap margin_visual_end
		if (margin_visual_end > sz)
			margin_visual_end = sz;
		int32_t margin_logical_end = vis2log_map[margin_visual_end];

		if (length <= margin_visual_end) {
			final_logical_end = sz;
			reached_end = true;
		} else if (is_it_line_empty) {
			final_logical_end = margin_logical_end;
		} else {
			UChar c = conf->utf16_str[margin_logical_end];
			int32_t cand_logical_end = margin_logical_end;
			int32_t cand_visual_end = margin_visual_end;

			// skip whitespace
			while (cand_visual_end < length && (u_isspace(c) || u_charType(c) == U_CONTROL_CHAR ||
							    u_charType(c) == U_NON_SPACING_MARK)) {
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
			(int32_t)(0.75f * (float)conf->max_line_width_chars) &&
		    !is_it_line_empty && !reached_end)
			final_logical_end = margin_logical_end;

		if (!mem_run) {
			arr_start[lines].start = line.start + final_logical_start;
			arr_start[lines].end = line.start + final_logical_end;
		}
		logical_start = final_logical_end;
		logical_end = final_logical_end;
		lines++;
	}

	*cnt = lines;

clean_wrap_mem:
	free(vis2log_map);
	free(log2vis_map);

clean_wrap_bidi:
	ubidi_close(bidi_line);
clean_it_bidi:
	ubrk_close(it_line);
no_clean:;
}

void __text_renderer_calculate_line_char_width(text_render_config * const conf)
{
	if (conf->it_char)
		return;
	UErrorCode u_error = U_ZERO_ERROR;

	conf->it_char = ICU_CHECK_FAIL(
	    ubrk_open(UBRK_CHARACTER, uloc_getDefault(), conf->utf16_str, conf->utf16_sz, &u_error), no_clean);

	conf->char_num = 0;
	int32_t logical = 0;
	while (logical != UBRK_DONE) {
		logical = ubrk_next(conf->it_char);
		conf->char_num++;
	}
	(void)ubrk_first(conf->it_char);

no_clean:;
}

void __text_renderer_get_line_break(text_render_config * const conf, int32_t line_number, int32_t * out_logical_start,
				    int32_t * out_logical_end)
{
	if (0 == conf->lines_cnt) {
		*out_logical_start = 0;
		*out_logical_end = conf->utf16_sz;
		return;
	}
	*out_logical_start = conf->lines[line_number].start;
	*out_logical_end = conf->lines[line_number].end;
}

void __text_renderer_new_line(text_render_config * const conf)
{
	// https://stackoverflow.com/questions/28009564/new-line-pixel-distance-in-freetype
	conf->curr_y -= (fonts_man_get_font_by_type(conf->renderer->font_style, 0)->face->size->metrics.height >> 6) +
			conf->spacing;
	conf->curr_x = conf->origin_x;
}

void text_renderer_update_window_size(text_renderer_t * renderer, int width, int height)
{
	renderer->scr_width = width;
	renderer->scr_height = height;

	glm_ortho(0.0f, (float)width, 0.0f, (float)height, -1, 1, renderer->projection);
}
