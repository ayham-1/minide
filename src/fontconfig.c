#include "minide/fontconfig.h"

#include "minide/logger.h"

#include <assert.h>
#include <fontconfig/fontconfig.h>
#include <malloc.h>

#define INITIAL_PATS_SIZE 10
#define MAX_FONT_SET_SIZE 5

static fc_state fc;

void fc_init()
{
	assert(!fc.config);

	fc.config = FcInitLoadConfigAndFonts();
	fc.sz = 0;
	fc.capacity = 0;

	log_info("initialized fontconfig library of version %i", FcGetVersion());
}

void fc_clean()
{
	assert(fc.config);

	for (size_t i = 0; i < fc.capacity; i++) {
		FcFontSetDestroy(fc.list[i].matched_fonts);
	}

	FcConfigDestroy(fc.config);
	FcFini();
}

fc_holder * fc_request(char * font_name)
{
	if (fc.sz == 0 || fc.capacity + 1 >= fc.sz) {
		fc.sz += INITIAL_PATS_SIZE;
		fc.list = realloc(fc.list, sizeof(fc_holder) * fc.sz);
	}

	FcPattern * pat = fc.list[fc.capacity].pattern;
	pat = FcNameParse((const FcChar8 *)font_name);

	FcConfigSubstitute(fc.config, pat, FcMatchPattern);
	FcDefaultSubstitute(pat);

	FcResult result;
	fc.list[fc.capacity].matched_fonts = FcFontSort(fc.config, pat, FcTrue, NULL, &result);

	if (result != FcResultMatch) {
		log_error("failed to resolve font name pattern: %s", font_name);
		switch (result) {
		case FcResultNoMatch:
			log_error("error: FcResultNoMatch");
			break;
		case FcResultTypeMismatch:
			log_error("error: FcResultTypeMismatch");
			break;
		case FcResultNoId:
			log_error("error: FcResultNoId");
			break;
		case FcResultOutOfMemory:
			log_error("error: FcResultOutOfMemory");
			break;
		default:
			log_error("unknown error");
		};
		return NULL;
	}

	fc.list[fc.capacity].matched_fonts_n = (fc.list[fc.capacity].matched_fonts->nfont <= MAX_FONT_SET_SIZE)
						   ? fc.list[fc.capacity].matched_fonts->nfont
						   : MAX_FONT_SET_SIZE;

	FcFontSet * font_set = fc.list[fc.capacity].matched_fonts;
	size_t matched_fonts_n = fc.list[fc.capacity].matched_fonts_n;

	fc.list[fc.capacity].matched_fonts_paths = calloc(matched_fonts_n, sizeof(FcChar8 *));
	for (int i = 0; i < matched_fonts_n; i++) {
		FcChar8 * file = NULL;
		if (FcPatternGetString(font_set->fonts[i], FC_FILE, 0, &file) != FcResultMatch) {
			log_error("unable to get file path from already matched file pattern");
			file = NULL;
		}
		fc.list[fc.capacity].matched_fonts_paths[i] = file;
	}
	return &fc.list[fc.capacity++];
}

char * fc_get_path_by_font(fc_holder * holder)
{
	assert(holder);

	return (char *)holder->matched_fonts_paths[0];
}

char * fc_get_path_by_font_order(fc_holder * holder, size_t n)
{
	assert(holder);
	assert(n < holder->matched_fonts_n);

	return (char *)holder->matched_fonts_paths[n];
}
