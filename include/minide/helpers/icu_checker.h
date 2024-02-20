#ifndef ICU_CHECKER_H
#define ICU_CHECKER_H

#define ICU_CHECK_FAIL(cmd, cleanup_where)                                                                             \
	cmd;                                                                                                           \
	if (U_FAILURE(u_error)) {                                                                                      \
		log_error("function: %s,\n\t\t\t\t\tu_error: %s", #cmd, u_errorName(u_error));                         \
		goto cleanup_where;                                                                                    \
	}

#endif
