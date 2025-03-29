#ifndef WEFT_PARSE_H
#define WEFT_PARSE_H

#include <stddef.h>

// Forward Declarations

typedef struct weft_buf Weft_Buf;
typedef struct weft_list Weft_List;
typedef struct weft_map Weft_Map;
typedef struct weft_parse_state Weft_ParseState;

// Constants

static const size_t WEFT_PARSE_INIT_BUF_CAP = 1;

// Data Types

enum weft_parse_shuffle_mode {
	WEFT_PARSE_SHUFFLE_OFF,
	WEFT_PARSE_SHUFFLE_IN,
	WEFT_PARSE_SHUFFLE_OUT,
};

struct weft_parse_state {
	const char *path;
	const char *start;

	Weft_Map *map;
	Weft_Buf *map_stack;
	Weft_Buf *def_stack;

	Weft_List *list;
	Weft_List *node;
	Weft_Buf *list_stack;
	size_t nest;

	size_t indent;
	Weft_Buf *indent_stack;

	enum weft_parse_shuffle_mode shuffle_mode;
	Weft_Buf *shuffle_in;
	Weft_Buf *shuffle_out;
};

// Functions

void parse_init(Weft_ParseState *P);
void parse_exit(Weft_ParseState *P);
Weft_List *parse(Weft_ParseState *P, const char *path, const char *src);

#endif
