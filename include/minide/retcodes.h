#ifndef MIDE_RETCODES_H
#define MIDE_RETCODES_H

#include <stdbool.h>
#include <stdint.h>

#define MIDE_SUCCESS 0L

#define MIDE_NEW_RET_TYPE(RET) {"#RET"}

#define MIDE_GENERIC_FAILURE 1L

#define MIDE_MEMORY_FAILURE 2L
#define MIDE_LOGIC_FAILURE 3L
#define MIDE_IO_FAILURE 4L
#define MIDE_INVALID_ARGUMENT 5L
#define MIDE_INVALID_STATE 6L
#define ___MIDE_LAST_RETCODE MIDE_INVALID_STATE // for implementation use only

const char * mide_error_names[] = {"MIDE_SUCCESS", "MIDE_LOGIC_FAILURE", "MIDE_IO_FAILURE", "MIDE_INVALID_ARGUMENT",
				   "MIDE_INVALID_STATE"};

void mide_register_retcode(uint64_t ret_code);
void mide_unregister_retcode(uint64_t ret_code);
void mide_reset_retcodes();

bool mide_was_success();
uint64_t mide_get_last_error();

#define MIDE_IS_RET_VALID(ret_code) (ret_code >= MIDE_SUCCESS && ret_code <= ___MIDE_LAST_RETCODE)

#endif
