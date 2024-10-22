#include "minide/retcodes.h"

#include <assert.h>

static bool error_state[___MIDE_LAST_RETCODE] = {0};
static uint64_t last_error = MIDE_SUCCESS;

void mide_register_retcode(uint64_t ret_code)
{
	assert(MIDE_IS_RET_VALID(ret_code));
	error_state[ret_code] = true;
	last_error = ret_code;
}
void mide_unregister_retcode(uint64_t ret_code)
{
	assert(MIDE_IS_RET_VALID(ret_code));
	error_state[ret_code] = false;
	if (last_error == ret_code) {
		last_error = MIDE_SUCCESS;
	}
}
void mide_reset_retcodes()
{
	for (int i = 0; i < ___MIDE_LAST_RETCODE; i++) {
		error_state[i] = false;
	}
}

bool mide_was_success()
{
	if (error_state[MIDE_SUCCESS]) {
		error_state[MIDE_SUCCESS] = false;
		return true;
	}
	return false;
}
