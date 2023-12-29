#ifndef LOGGER_H
#define LOGGER_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
	FILE * (*fopen)(char const * const filename, char const * const mode);
	int (*fprintf)(FILE * fp, char const * const fmt, ...);
	int (*vfprintf)(FILE * fp, char const * const fmt, va_list argp);
} std_file_t;

void std_file_init(std_file_t * const std_file);

typedef enum { OFF, DEBUG, INFO, WARNING, ERROR } logger_level_t;

typedef struct {
	logger_level_t level;
	FILE * fp;
	std_file_t * std_file;
} logger_t;

int logger_init(logger_level_t const level, char const * const filename,
		bool const write_to_file);
void logger_cleanup();

void logger_log(logger_level_t const level, char const * const file,
		size_t const line, char const * const fmt, ...);

extern logger_t * g_logger;

#define __FILENAME__                                                           \
	(strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define log_debug(...) logger_log(DEBUG, __FILE_NAME__, __LINE__, __VA_ARGS__)
#define log_info(...) logger_log(INFO, __FILE_NAME__, __LINE__, __VA_ARGS__)
#define log_warn(...) logger_log(WARNING, __FILE_NAME__, __LINE__, __VA_ARGS__)
#define log_error(...) logger_log(ERROR, __FILE_NAME__, __LINE__, __VA_ARGS__)

#define log_var(var)                                                           \
	logger_log(DEBUG, __FILE_NAME__, __LINE__, "%s: %i", #var, var)
#endif
