#include "shaper.h"

#include <assert.h>

shaper_holder shaper_do(UChar* utf16_str,
                        int32_t logical_length,
                        enum FontFamilyStyle preferred_style,
                        size_t pixel_size,
                        bool do_style_fallback,
                        bool do_font_fallback) {
    shaper_holder holder;
    assert(utf16_str);

    holder.preferred_style = preferred_style;
    holder.do_style_falllback = do_style_fallback;
    holder.do_font_fallback = do_font_fallback;

    holder.runs_capacity = 1;
    holder.runs_fullness = 0;
    holder.runs = calloc(holder.runs_capacity, sizeof(shaper_font_run_t));

    font_t* primary_font = fonts_man_get_font_by_type(preferred_style);
    holder.buffer = hb_buffer_create();
    assert(hb_buffer_allocation_successful(holder.buffer));

    hb_buffer_reset(holder.buffer);
    hb_buffer_clear_contents(holder.buffer);
    hb_buffer_add_utf16(holder.buffer, (uint16_t*)utf16_str, logical_length, 0, -1);

    hb_buffer_guess_segment_properties(holder.buffer);

    font_set_pixel_size(primary_font, pixel_size);
    hb_shape(primary_font->hb, holder.buffer, NULL, 0);

    // do primary font runs
    //
    unsigned int glyph_count;
    hb_glyph_info_t* glyph_infos = hb_buffer_get_glyph_infos(holder.buffer, &glyph_count);
    hb_glyph_position_t* glyph_pos = hb_buffer_get_glyph_positions(holder.buffer, &glyph_count);

    unsigned int glyph_start = 0;
    unsigned int glyph_end = 0;
    int32_t logical_start = glyph_infos[0].cluster;
    int32_t logical_end = 0;
    for (unsigned int i = glyph_start; i < glyph_count; i++) {
        if (glyph_infos[i].codepoint == 0 || i == glyph_count - 1) {
            glyph_end = i;
            logical_end = glyph_infos[i].cluster;
            __shaper_add_run(&holder, 
                             __shaper_make_run(logical_start, logical_end,
                                               primary_font,
                                               &glyph_infos[glyph_start],
                                               &glyph_pos[glyph_start],
                                               glyph_end - glyph_start));
            glyph_start = i;
        } else {
            continue;
        }
    }

    return holder;
}

void shaper_undo(shaper_holder* holder) {
    for (size_t i = 0; i < holder->runs_fullness; i++) {
        __shaper_clean_run(&holder->runs[i]);
    }

    hb_buffer_destroy(holder->buffer);
}

void __shaper_add_run(shaper_holder* holder, shaper_font_run_t run) {
    if (holder->runs_fullness + 1 >= holder->runs_capacity) {
        holder->runs = (shaper_font_run_t*) realloc(holder->runs, 2 * holder->runs_capacity * sizeof(shaper_font_run_t));
        holder->runs_capacity *= 2;
        log_info("shaper runs full, attempted realloc");
    }

    holder->runs[holder->runs_fullness++] = run;
}

shaper_font_run_t __shaper_make_run(int32_t logical_start, int32_t logical_end,
                                    font_t* font, 
                                    hb_glyph_info_t* info_start,
                                    hb_glyph_position_t* pos_start,
                                    size_t glyph_count) {
    shaper_font_run_t run;

    run.logical_start = logical_start;
    run.logical_end = logical_end;
    run.font = font;
    run.glyph_count = glyph_count;

    run.glyph_infos = (hb_glyph_info_t*) calloc(run.glyph_count, sizeof(hb_glyph_info_t));
    run.glyph_pos = (hb_glyph_position_t*) calloc(run.glyph_count, sizeof(hb_glyph_position_t));

    memcpy(run.glyph_infos, info_start, glyph_count * sizeof(hb_glyph_info_t));
    memcpy(run.glyph_pos, pos_start, glyph_count * sizeof(hb_glyph_position_t));

    return run;
}

void __shaper_clean_run(shaper_font_run_t* run) {
    free(run->glyph_infos);
    free(run->glyph_pos);
    run->glyph_count = 0;
    run->logical_start = 0;
    run->logical_end = 0;
}
