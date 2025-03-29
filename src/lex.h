#ifndef WEFT_LEX_H
#define WEFT_LEX_H

#include <stdbool.h>
#include <stddef.h>

// Forward Declarations

typedef struct weft_str Weft_Str;
typedef struct weft_lex Weft_Lex;
typedef enum weft_lex_type Weft_LexType;

// Data Types

enum weft_lex_type {
	WEFT_LEX_INDENT,
	WEFT_LEX_EMPTY,
	WEFT_LEX_NUM,
	WEFT_LEX_CHAR,
	WEFT_LEX_STR,
	WEFT_LEX_QUOTATION_OPEN,
	WEFT_LEX_QUOTATION_CLOSE,
	WEFT_LEX_SHUFFLE_OPEN,
	WEFT_LEX_SHUFFLE_CLOSE,
	WEFT_LEX_WORD,
	WEFT_LEX_DEF,
};

struct weft_lex {
	const char *src;
	size_t len;
	Weft_LexType type;
	union {
		double num;
		unsigned char c;
		Weft_Str *str;
		void *ptr;
	};
};

// Functions

void lex_error(const char *path,
               const char *start,
               const char *src,
               size_t len,
               const char *fmt,
               ...);
bool lex_is_line_empty(const char *src);
Weft_Lex lex_indent(const char *src);
Weft_Lex lex_comment(const char *src);
Weft_Lex lex_empty(const char *src);
Weft_Lex lex_num(const char *path, const char *start, const char *src);
Weft_Lex lex_quotation_open(const char *src);
Weft_Lex lex_quotation_close(const char *src);
Weft_Lex lex_shuffle_open(const char *src);
Weft_Lex lex_shuffle_close(const char *src);
Weft_Lex lex_word(const char *src);
Weft_Lex lex_token(const char *path, const char *start, const char *src);

#endif
