#ifndef WEFT_PARSE_H
#define WEFT_PARSE_H

// Forward Declarations

typedef struct weft_parse_state Weft_ParseState;
typedef struct weft_buf Weft_Buf;
typedef struct weft_list Weft_List;
typedef struct weft_map Weft_Map;

// Local Includes

#include "word.h"

// Data Types

struct weft_parse_state {
	const char *src;

	Weft_Word word;
	Weft_Buf *def_stack;

	size_t indent;
	Weft_Buf *indent_stack;

	Weft_List *list;
	Weft_List *node;
	Weft_Buf *list_stack;
	size_t nest;

	Weft_Map *map;
	Weft_Buf *map_stack;
};

// Constants

static const size_t WEFT_PARSE_INIT_STACK_CAP = 1;

// Functions

void parse_init(Weft_ParseState *P);
void parse_exit(Weft_ParseState *P);
void parse_mark(Weft_ParseState *P);
void parse_error(const char *src, size_t at, const char *fmt, ...);
void parse_error_v(const char *src, size_t at, const char *fmt, va_list args);
Weft_List *parse(Weft_ParseState *P, const char *src);

#endif
