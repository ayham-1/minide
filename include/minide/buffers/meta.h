#ifndef BUFFER_META_H
#define BUFFER_META_H

#include <unicode/uchar.h>

typedef struct {
	bool text;
	bool picture;
	bool editable;
	bool code;
	bool terminal;
} buffer_abilities;

enum BufferHandlerType {
	BUFFER_HANDLER_Nodes,
	BUFFER_HANDLER_Terminal,
};

typedef struct {
	UChar * name;
	size_t line;
	size_t current_line;
	size_t current_coln;

	void * func_init;
	void * func_clean;

	void * handler;
	enum BufferHandlerType handler_type;
} buffer_meta;

typedef bool (*buffer_init)(buffer_meta * meta);
typedef void (*buffer_clean)(buffer_meta * meta);

#endif
