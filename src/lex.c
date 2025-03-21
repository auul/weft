#include "lex.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

// Constants

static const char delim_list[] = "#[]:";

// Functions

static Weft_LexData tag_data(const char *src, size_t len, Weft_LexType type)
{
	Weft_LexData data = {
		.src = src,
		.len = len,
		.type = type,
	};
	return data;
}

const char *lex_src(const Weft_LexData data)
{
	return data.src;
}

size_t lex_len(const Weft_LexData data)
{
	return data.len;
}

Weft_LexType lex_type(const Weft_LexData data)
{
	return data.type;
}

Weft_Word lex_data_to_word(const Weft_LexData data)
{
	Weft_Word word = {
		.src = data.src,
		.len = data.len,
	};
	return word;
}

void lex_data_print(const Weft_LexData data)
{
	switch (data.type) {
	case WEFT_LEX_EMPTY:
		printf("empty(%zu)", data.len);
		break;
	case WEFT_LEX_INDENT:
		if (data.len == 0) {
			printf("indent(%zu)", data.len);
		} else {
			printf("indent(%zu)", data.len - 1);
		}
		break;
	case WEFT_LEX_WORD:
		printf("%.*s", data.len, data.src);
		break;
	case WEFT_LEX_OPEN_LIST:
		printf("OPEN");
		break;
	case WEFT_LEX_CLOSE_LIST:
		printf("CLOSE");
		break;
	case WEFT_LEX_DEFINE:
		printf("DEF");
		break;
	default:
		break;
	}
}

static bool is_delim(const char *src)
{
	return !*src || isspace(*src) || strchr(delim_list, *src);
}

bool lex_is_line_empty(const char *src)
{
	for (size_t i = 0; src[i] && src[i] != '\n'; i++) {
		if (src[i] == '#') {
			return true;
		} else if (!isspace(src[i])) {
			return false;
		}
	}
	return true;
}

Weft_LexData lex_empty(const char *src)
{
	size_t len = 0;
	while (true) {
		if (!src[len]) {
			return tag_data(src, len, WEFT_LEX_EMPTY);
		} else if (src[len] == '\n') {
			if (lex_is_line_empty(src + len + 1)) {
				len++;
				while (src[len] && src[len] != '\n') {
					len++;
				}
			} else {
				return tag_data(src, len, WEFT_LEX_EMPTY);
			}
		} else if (src[len] == '#') {
			while (src[len] && src[len] != '\n') {
				len++;
			}
		} else if (isspace(src[len])) {
			len++;
		} else {
			return tag_data(src, len, WEFT_LEX_EMPTY);
		}
	}
}

Weft_LexData lex_indent(const char *src)
{
	size_t len = 0;
	while (isspace(src[len])) {
		len++;
	}
	return tag_data(src, len, WEFT_LEX_INDENT);
}

static Weft_LexData lex_word(const char *src)
{
	size_t len = 0;
	while (!is_delim(src + len)) {
		len++;
	}
	return tag_data(src, len, WEFT_LEX_WORD);
}

Weft_LexData lex_token(const char *src)
{
	switch (src[0]) {
	case '\n':
		return lex_indent(src);
	case '[':
		return tag_data(src, 1, WEFT_LEX_OPEN_LIST);
	case ']':
		return tag_data(src, 1, WEFT_LEX_CLOSE_LIST);
	case ':':
		return tag_data(src, 1, WEFT_LEX_DEFINE);
	default:
		return lex_word(src);
	}
}
