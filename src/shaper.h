#ifndef SHAPER_H
#define SHAPER_H

#include "logger.h"

#include "font.h"
#include "font_manager.h"

#include <unicode/uchar.h>
#include <harfbuzz/hb.h>

typedef struct {
    int32_t logical_start;
    int32_t logical_end;

    font_t* font;
    float scale;
    bool is_textual_single_color;

    hb_glyph_info_t* glyph_infos;
    hb_glyph_position_t* glyph_pos;
    size_t glyph_count;
} shaper_font_run_t;

typedef struct {
    enum FontFamilyStyle preferred_style;
    bool do_style_falllback;
    bool do_font_fallback;

    shaper_font_run_t* runs;
    size_t runs_capacity;
    size_t runs_fullness;

    hb_buffer_t* buffer;
} shaper_holder;

shaper_holder shaper_do(UChar* utf16_str,
                        int32_t logical_length,
                        enum FontFamilyStyle preferred_style,
                        size_t pixel_size,
                        bool do_style_fallback,
                        bool do_font_fallback);
void shaper_undo(shaper_holder* holder);

void __shaper_add_run(shaper_holder* holder, shaper_font_run_t run);
shaper_font_run_t __shaper_make_run(int32_t logical_start, int32_t logical_end,
                                    font_t* font, 
                                    hb_glyph_info_t* info_start,
                                    hb_glyph_position_t* pos_start,
                                    size_t glyph_count);
void __shaper_clean_run(shaper_font_run_t* run);

#endif
