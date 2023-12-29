#include "fontconfig.h"

#include "logger.h"

#include <assert.h>
#include <malloc.h>

#define INITIAL_PATS_SIZE 10

static fc_state fc;

void fc_init()
{
	assert(!fc.config);

	fc.config = FcInitLoadConfigAndFonts();
	fc.sz = 0;
	fc.capacity = 0;

	log_info("initialized fontconfig library of version %i",
		 FcGetVersion());
}

void fc_clean()
{
	assert(fc.config);

	for (size_t i = 0; i < fc.capacity; i++) {
		// FcPatternDestroy(fonts.list[i].pattern);
		FcPatternDestroy(fc.list[i].matched_font);
	}

	FcConfigDestroy(fc.config);
	FcFini();
}

fc_holder * fc_request(char * font_name)
{
	if (fc.sz == 0) {
		fc.sz = INITIAL_PATS_SIZE;
		fc.list = malloc(sizeof(fc_holder) * fc.sz);
	}

	if (fc.capacity + 1 >= fc.sz) {
		fc.sz += INITIAL_PATS_SIZE;
		fc.list = realloc(fc.list, sizeof(fc_holder) * fc.sz);
	}

	FcPattern * pat = fc.list[fc.capacity].pattern;
	pat = FcNameParse((const FcChar8 *)font_name);

	FcConfigSubstitute(fc.config, pat, FcMatchPattern);
	FcDefaultSubstitute(pat);

	FcResult result;
	fc.list[fc.capacity].matched_font =
	    FcFontMatch(fc.config, pat, &result);

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

	FcChar8 * file = NULL;
	if (FcPatternGetString(fc.list[fc.capacity].matched_font, FC_FILE, 0,
			       &file) != FcResultMatch) {
		log_error("unable to get file path from already matched file "
			  "pattern");
		file = NULL;
	}
	fc.list[fc.capacity].matched_font_path = file;
	return &fc.list[fc.capacity++];
}

char * fc_get_path_by_font(fc_holder * holder)
{
	assert(holder);

	return (char *)holder->matched_font_path;
}
