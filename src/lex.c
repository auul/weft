#include "lex.h"
#include "error.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

// Constants

static const char delim_list[] = "#[]{}:";

// Functions

#define len_of(str) (sizeof(str) - 1)

static Weft_Lex tag_lex(const char *src, size_t len, Weft_LexType type)
{
	Weft_Lex lex = {
		.src = src,
		.len = len,
		.type = type,
	};
	return lex;
}

static Weft_Lex tag_indent(const char *src, size_t len)
{
	return tag_lex(src, len, WEFT_LEX_INDENT);
}

static Weft_Lex tag_empty(const char *src, size_t len)
{
	return tag_lex(src, len, WEFT_LEX_EMPTY);
}

static Weft_Lex tag_num(const char *src, size_t len)
{
	return tag_lex(src, len, WEFT_LEX_NUM);
}

bool lex_is_line_empty(const char *src)
{
	while (true) {
		if (!*src || *src == '\n' || *src == '#') {
			return true;
		} else if (isspace(*src)) {
			src++;
		} else {
			return false;
		}
	}
}

static bool is_indent(const char *src)
{
	return src[0] == '\n' && !lex_is_line_empty(src + 1);
}

Weft_Lex lex_indent(const char *src)
{
	size_t len = 0;
	while (isspace(src[len])) {
		len++;
	}
	return tag_indent(src, len);
}

static bool is_comment(const char *src)
{
	return src[0] == '#';
}

Weft_Lex lex_comment(const char *src)
{
	size_t len = len_of("#");
	while (src[len] && src[len] != '\n') {
		len++;
	}
	return tag_empty(src, len);
}

Weft_Lex lex_empty(const char *src)
{
	size_t len = 0;
	while (true) {
		if (is_comment(src + len)) {
			len += lex_comment(src + len).len;
		} else if (is_indent(src + len)) {
			return tag_empty(src, len);
		} else if (isspace(src[len])) {
			len++;
		} else {
			return tag_empty(src, len);
		}
	}
}

static bool is_delim(const char *src)
{
	return !*src || isspace(*src) || strchr(delim_list, *src);
}

static bool is_num(const char *src)
{
	if (*src == '-') {
		src++;
	}

	if (*src == '.') {
		src++;
	}
	return isdigit(*src);
}

Weft_Lex lex_num(const char *path, const char *start, const char *src)
{
	size_t len = 0;
	if (src[len] == '-') {
		len++;
	}

	bool dot = false;

	while (src[len] == '.' || isdigit(src[len])) {
		if (src[len] == '.') {
			if (dot) {
				error_parse(
					path, start, src + len, 1, "Extra . in number literal");
			}
			dot = true;
		}
		len++;
	}

	if (!is_delim(src + len)) {
		error_parse(
			path, start, src + len, 1, "End of number literal expected");
		while (!is_delim(src + len)) {
			len++;
		}
	} else if (src[len - 1] == '.') {
		error_parse(path,
		            start,
		            src + len - 1,
		            1,
		            "At least one digit expected after '.'");
	}
	return tag_num(src, len);
}

static bool is_quotation_open(const char *src)
{
	return src[0] == '[';
}

Weft_Lex lex_quotation_open(const char *src)
{
	return tag_lex(src, len_of("["), WEFT_LEX_QUOTATION_OPEN);
}

static bool is_quotation_close(const char *src)
{
	return src[0] == ']';
}

Weft_Lex lex_quotation_close(const char *src)
{
	return tag_lex(src, len_of("]"), WEFT_LEX_QUOTATION_CLOSE);
}

static bool is_shuffle_open(const char *src)
{
	return src[0] == '{';
}

Weft_Lex lex_shuffle_open(const char *src)
{
	return tag_lex(src, len_of("{"), WEFT_LEX_SHUFFLE_OPEN);
}

static bool is_shuffle_close(const char *src)
{
	return src[0] == '}';
}

Weft_Lex lex_shuffle_close(const char *src)
{
	return tag_lex(src, len_of("}"), WEFT_LEX_SHUFFLE_CLOSE);
}

Weft_Lex lex_word(const char *src)
{
	size_t len = 0;
	while (!is_delim(src + len)) {
		len++;
	}

	if (src[len] == ':') {
		return tag_lex(src, len + 1, WEFT_LEX_DEF);
	}
	return tag_lex(src, len, WEFT_LEX_WORD);
}

Weft_Lex lex_token(const char *path, const char *start, const char *src)
{
	if (src[0] == ':') {
		error_parse(path, start, src, 1, "Expected word before ':'");
		return tag_empty(src, 1);
	} else if (is_indent(src)) {
		return lex_indent(src);
	} else if (is_num(src)) {
		return lex_num(path, start, src);
	} else if (is_quotation_open(src)) {
		return lex_quotation_open(src);
	} else if (is_quotation_close(src)) {
		return lex_quotation_close(src);
	} else if (is_shuffle_open(src)) {
		return lex_shuffle_open(src);
	} else if (is_shuffle_close(src)) {
		return lex_shuffle_close(src);
	} else {
		return lex_word(src);
	}
}
