#include "minide/logger.h"

#include <string.h>
#include <time.h>
#include <unistd.h>

logger_t * g_logger;

void std_file_init(std_file_t * const std_file)
{
	std_file->fopen = fopen;
	std_file->fprintf = fprintf;
	std_file->vfprintf = vfprintf;
}

int logger_init(logger_level_t const level, char const * const filename,
		bool const write_to_file)
{
	std_file_t * std_file = malloc(sizeof(std_file_t));
	g_logger = malloc(sizeof(logger_t));

	g_logger->level = level;
	std_file_init(std_file);
	g_logger->std_file = std_file;

	if (write_to_file) {
		g_logger->fp = g_logger->std_file->fopen(filename, "a");
		if (g_logger->fp == NULL) {
			return -1;
		}
	} else {
		g_logger->fp = NULL;
	}

	log_info("logger started");

	return 0;
}
void logger_cleanup()
{
	log_info("cleaning logger...");
	free(g_logger->std_file);
	if (g_logger->fp != NULL) {
		fclose(g_logger->fp);
	}
	free(g_logger);
}

static const char * log_level_strings[] = {
    "OFF", "[DEBUG]", "[INFO]", "[WARNING]", "[ERROR]",
};

#define RESET 0
const char * colors[] = {
    "\x1B[0m", "\x1B[34m", "\x1B[32m", "\x1B[33m", "\x1B[31m",
};

static void get_datetime(FILE * fptr)
{
	time_t t = time(NULL);
	struct tm * tm = localtime(&t);
	const char * d_time = asctime(tm);
	const char * datetime = strchr(d_time, '\n');
	int length = datetime - d_time;

	char * modified_date = malloc(length + 1);
	strncpy(modified_date, d_time, length);
	modified_date[length] = '\0';
	fprintf(fptr, "%s ", modified_date);

	free(modified_date);
}

void logger_log(logger_level_t const level, char const * const file,
		size_t const line, char const * const fmt, ...)
{
	if (level < g_logger->level) {
		return;
	}

	va_list argp;
	// file logger
	if (g_logger->fp != NULL) {
		get_datetime(g_logger->fp);
		g_logger->std_file->fprintf(g_logger->fp, "%-10s",
					    log_level_strings[level]);
		g_logger->std_file->fprintf(g_logger->fp, "%s: %lu: ", file,
					    line);
		va_start(argp, fmt);
		g_logger->std_file->vfprintf(g_logger->fp, fmt, argp);
		va_end(argp);
		g_logger->std_file->fprintf(g_logger->fp, "\n");
		fflush(g_logger->fp);
	}

	// console output
	FILE * fp = NULL;
	switch (level) {
	case OFF:
		return;
	case DEBUG:
	case INFO:
	case WARNING:
	case ERROR:
		fp = stdout; // logging by ">" or "|" (for some weird reason,
			     // idk)
	};

	get_datetime(fp);
	fprintf(fp, "%s%-10s%s", colors[level], log_level_strings[level],
		colors[RESET]);
	fprintf(fp, "%s: %lu: ", file, line);
	va_start(argp, fmt);
	vfprintf(fp, fmt, argp);
	fprintf(fp, "\n");
}
