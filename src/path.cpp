extern "C" {
#include "minide/path.h"
}

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <filesystem>
#include <fstream>
namespace fs = std::filesystem;

bool path_create_file(char * p)
{
	assert(p);
	try {
		std::ofstream o(p);
		o.close();
		return true;
	} catch (std::exception const & e) {
		log_error("std::ofstream file creation failed with error: %s", e.what());
		return false;
	}
}

bool path_create_dir(char * p)
{
	assert(p);
	try {
		return fs::create_directory(p);
	} catch (std::exception const & e) {
		log_error("std::filesystem::create_directory() failed with error: %s", e.what());
		return false;
	}
}

bool path_is_real(char * p)
{
	const fs::path path{p};
	return fs::exists(p);
}

bool path_can_read(char * p)
{
	assert(p);
	try {
		std::ifstream file(p);
		bool res = file.good();
		file.close();
		return res;
	} catch (std::exception const & e) {
		log_warn("path_can_read() failed with error: %s, returning no access", e.what());
		return false;
	}
}
char * path_get_name(char * p)
{
	assert(p);
	try {
		fs::path path{p};
		std::string result = path.filename().u8string() + '\0';
		char * c_result = (char *)malloc(result.length());
		memcpy(c_result, result.c_str(), result.length());
		return c_result;
	} catch (std::exception const & e) {
		log_error("path_get_name() failed with error: %s", e.what());
		return NULL;
	}
}

char * path_get_extension(char * p)
{
	assert(p);
	try {
		fs::path path{p};
		std::string result = path.extension().u8string() + '\0';
		char * c_result = (char *)malloc(result.length());
		memcpy(c_result, result.c_str(), result.length());
		return c_result;
	} catch (std::exception const & e) {
		log_error("path_get_extension() failed with error: %s", e.what());
		return NULL;
	}
}

char * path_get_absolute(char * p)
{
	assert(p);
	try {
		fs::path path{p};
		std::string result = fs::absolute(path).u8string() + '\0';
		char * c_result = (char *)malloc(result.length());
		memcpy(c_result, result.c_str(), result.length());
		return c_result;
	} catch (std::exception const & e) {
		log_error("path_get_absolute() failed with error: %s", e.what());
		return NULL;
	}
}

file_type_t path_get_type(char * p)
{
	assert(p);
	try {
		fs::path path{p};
		fs::file_status s = fs::status(p);
		switch (s.type()) {
		case fs::file_type::none:
			return file_type_none;
		case fs::file_type::not_found:
			return file_type_not_found;
		case fs::file_type::regular:
			return file_type_regular;
		case fs::file_type::directory:
			return file_type_directory;
		case fs::file_type::symlink:
			return file_type_symlink;
		case fs::file_type::block:
			return file_type_block;
		case fs::file_type::character:
			return file_type_character;
		case fs::file_type::fifo:
			return file_type_fifo;
		case fs::file_type::socket:
			return file_type_socket;
		case fs::file_type::unknown:
			return file_type_unknown;
		default:
			return file_type_unknown;
		}
	} catch (std::exception const & e) {
		log_warn("path_get_type() failed with error: %s", e.what());
		return file_type_unknown;
	}
}
