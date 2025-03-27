#include "debug.h"

#ifndef WEFT_PARSE_H
#define WEFT_PARSE_H

#include <stdarg.h>
#include <stddef.h>

#ifdef WEFT_DEBUG_ALL
#define WEFT_DEBUG_PARSE
#endif

// Forward Declarations

typedef struct weft_parse_state Weft_ParseState;
typedef struct weft_map Weft_Map;
typedef struct weft_buf Weft_Buf;
typedef struct weft_list Weft_List;

// Data Types

struct weft_parse_state {
	char *start;

	Weft_Map *map;
	Weft_Buf *map_stack;

	const char *word;
	size_t word_len;
	Weft_Buf *def_stack;

	Weft_List *list;
	Weft_List *node;
	Weft_Buf *list_stack;
	size_t nest;

	size_t indent;
	Weft_Buf *indent_stack;

	unsigned shuffle_mode;
	Weft_Buf *shuffle_in;
	Weft_Buf *shuffle_out;
};

// Constants

static const unsigned WEFT_PARSE_SHUFFLE_OFF = 0;
static const unsigned WEFT_PARSE_SHUFFLE_IN = 1;
static const unsigned WEFT_PARSE_SHUFFLE_OUT = 2;

// Functions

#ifdef WEFT_DEBUG_PARSE
void parse_debug(const Weft_ParseState *P);
#endif

void parse_init(Weft_ParseState *P);
void parse_exit(Weft_ParseState *P);
void parse_error(const Weft_ParseState *P,
                 const char *src,
                 size_t len,
                 const char *fmt,
                 ...);
void parse_error_v(const Weft_ParseState *P,
                   const char *src,
                   size_t len,
                   const char *fmt,
                   va_list args);
void parse_mark(Weft_ParseState *P);
Weft_List *parse(Weft_ParseState *P, char *src);

#endif
