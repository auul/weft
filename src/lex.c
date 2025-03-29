#include "lex.h"
#include "buf.h"
#include "char.h"
#include "error.h"
#include "str.h"

#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
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

static Weft_Lex tag_num(const char *src, size_t len, double num)
{
	Weft_Lex lex = {
		.src = src,
		.len = len,
		.type = WEFT_LEX_NUM,
		.num = num,
	};
	return lex;
}

static Weft_Lex tag_char(const char *src, size_t len, unsigned char c)
{
	Weft_Lex lex = {
		.src = src,
		.len = len,
		.type = WEFT_LEX_CHAR,
		.c = c,
	};
	return lex;
}

static Weft_Lex tag_str(const char *src, size_t len, Weft_Str *str)
{
	Weft_Lex lex = {
		.src = src,
		.len = len,
		.type = WEFT_LEX_STR,
		.str = str,
	};
	return lex;
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
	bool negative = false;
	double left = 0.0;
	double right = 0.0;
	unsigned place = 0;
	bool dot = false;

	size_t len;
	if (src[0] == '-') {
		negative = true;
		len = 1;
	} else {
		negative = false;
		len = 0;
	}

	while (src[len] == '.' || isdigit(src[len])) {
		if (src[len] == '.') {
			if (dot) {
				error_parse(
					path, start, src + len, 1, "Extra . in number literal");
			}
			dot = true;
		} else if (dot) {
			right = (10.0 * right) + (double)(src[len] - '0');
			place++;
		} else {
			left = (10.0 * left) + (double)(src[len] - '0');
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

	while (place) {
		right /= 10.0;
		place--;
	}

	if (negative) {
		return tag_num(src, len, -(left + right));
	}
	return tag_num(src, len, left + right);
}

static bool is_char(const char *src)
{
	return src[0] == '\'';
}

static bool is_hex_esc(const char *src)
{
	return src[0] == '\\' && (src[1] == 'x' || src[1] == 'X');
}

static bool is_nibble(char c)
{
	return (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F') || isdigit(c);
}

static unsigned get_nibble(char c)
{
	if (c >= 'a' && c <= 'f') {
		return c - 'a' + 10;
	} else if (c >= 'A' && c <= 'F') {
		return c - 'A' + 10;
	}
	return c - '0';
}

Weft_Lex lex_hex_esc(const char *path, const char *start, const char *src)
{
	size_t len = len_of("\\x");
	if (!src[len]) {
		return tag_char(src, len, 0);
	} else if (!is_nibble(src[len])) {
		error_parse(
			path, start, src, len + 1, "Invalid value for hex char escape");
		return tag_char(src, len + 1, 0);
	}

	unsigned value = get_nibble(src[len]);
	len++;

	if (is_nibble(src[len])) {
		value = (value << 4) | get_nibble(src[len]);
		len++;
	}
	return tag_char(src, len, value);
}

static bool is_decimal_esc(const char *src)
{
	return src[0] == '\\' && isdigit(src[1]);
}

static unsigned get_digit(char c)
{
	return c - '0';
}

static unsigned push_digit(unsigned value, char c)
{
	return (10 * value) + get_digit(c);
}

static Weft_Lex
lex_decimal_esc(const char *path, const char *start, const char *src)
{
	size_t len = len_of("\\");
	if (!src[len]) {
		return tag_char(src, len, 0);
	}

	unsigned value = get_digit(src[len]);
	len++;

	if (isdigit(src[len])) {
		value = push_digit(value, src[len]);
		len++;
	}

	if (isdigit(src[len])) {
		value = push_digit(value, src[len]);
		len++;
	}

	if (value > UCHAR_MAX) {
		error_parse(path,
		            start,
		            src,
		            len,
		            "Char escape literal exceeds max char value of %u",
		            UCHAR_MAX);
		value = 0;
	}
	return tag_char(src, len, value);
}

static bool is_char_esc(const char *src)
{
	return src[0] == '\\';
}

Weft_Lex lex_char_esc(const char *src)
{
	size_t len = len_of("\\");
	if (!src[len]) {
		return tag_char(src, len, 0);
	}
	return tag_char(src, len + 1, char_get_esc(src[len]));
}

static size_t get_char_esc_len(const char *src)
{
	size_t len = len_of("\\");
	if (!src[len]) {
		return len;
	}
	return len + 1;
}

Weft_Lex lex_char(const char *path, const char *start, const char *src)
{
	Weft_Lex ch;
	size_t len = len_of("'");
	if (is_hex_esc(src + len)) {
		ch = lex_hex_esc(path, start, src + len);
	} else if (is_decimal_esc(src + len)) {
		ch = lex_decimal_esc(path, start, src + len);
	} else if (is_char_esc(src + len)) {
		ch = lex_char_esc(src + len);
	} else if (src[len]) {
		ch = tag_char(src + len, 1, src[len]);
	}
	len += ch.len;

	if (!src[len]) {
		error_parse(path, start, src, len, "Missing terminating ' character");
		return tag_empty(src, len);
	} else if (src[len] != '\'') {
		error_parse(
			path, start, src + len, 1, "Multi-character character constant");
		while (src[len] && src[len] != '\'') {
			len++;
		}
		if (src[len] == '\'') {
			return tag_empty(src, len + len_of("'"));
		}
		return tag_empty(src, len);
	}
	return tag_char(src, len + len_of("'"), ch.c);
}

static bool is_str(const char *src)
{
	return src[0] == '"';
}

Weft_Lex lex_str(const char *path, const char *start, const char *src)
{
	size_t len = len_of("\"");
	Weft_Buf *buf = new_buf(sizeof(char));

	while (src[len] != '"') {
		Weft_Lex ch;
		if (!src[len]) {
			free(buf);
			error_parse(path, start, src, len, "Unclosed string literal");
			return tag_str(src, len, NULL);
		} else if (is_hex_esc(src + len)) {
			ch = lex_hex_esc(path, start, src + len);
		} else if (is_decimal_esc(src + len)) {
			ch = lex_decimal_esc(path, start, src + len);
		} else if (is_char_esc(src + len)) {
			ch = lex_char_esc(src + len);
		} else {
			ch = tag_char(src + len, 1, src[len]);
		}

		buf_push(&buf, &ch.c, sizeof(char));
		len += ch.len;
	}
	len += len_of("\"");

	Weft_Str *str = new_str_from_n(buf_get_raw(buf), buf_get_at(buf));
	free(buf);

	return tag_str(src, len, str);
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
	} else if (is_char(src)) {
		return lex_char(path, start, src);
	} else if (is_str(src)) {
		return lex_str(path, start, src);
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
