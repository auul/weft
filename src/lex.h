#ifndef WEFT_LEX_H
#define WEFT_LEX_H

#include <stdbool.h>
#include <stddef.h>

// Forward Declarations

typedef enum weft_lex_type Weft_LexType;
typedef struct weft_lex_data Weft_LexData;

// Local Includes

#include "word.h"

// Data Types

enum weft_lex_type {
	WEFT_LEX_EMPTY,
	WEFT_LEX_INDENT,
	WEFT_LEX_WORD,
	WEFT_LEX_OPEN_LIST,
	WEFT_LEX_CLOSE_LIST,
	WEFT_LEX_DEFINE,
};

struct weft_lex_data {
	const char *src;
	size_t len;
	Weft_LexType type;
};

// Functions

const char *lex_src(const Weft_LexData data);
size_t lex_len(const Weft_LexData data);
Weft_LexType lex_type(const Weft_LexData data);
Weft_Word lex_data_to_word(const Weft_LexData data);
void lex_data_print(const Weft_LexData data);
bool lex_is_line_empty(const char *src);
Weft_LexData lex_indent(const char *src);
Weft_LexData lex_empty(const char *src);
Weft_LexData lex_token(const char *src);

#endif
