#include "str.h"
#include "char.h"
#include "gc.h"

#include <stdio.h>
#include <string.h>

// Constants

static const char EMPTY_STR[] = "";

// Functions

#define str_and_len_of(str) str, sizeof(str) - 1

Weft_Str *new_str(size_t len)
{
	Weft_Str *str = gc_alloc(sizeof(Weft_Str) + len + 1);
	str->len = len;
	str->c[len] = 0;

	return str;
}

Weft_Str *new_str_from(const char *src)
{
	if (!src) {
		return NULL;
	}
	return new_str_from_n(src, strlen(src));
}

Weft_Str *new_str_from_n(const char *src, size_t len)
{
	Weft_Str *str = new_str(len);
	memcpy(str->c, src, len);

	return str;
}

size_t str_get_len(const Weft_Str *str)
{
	return str->len;
}

const char *str_get_c(const Weft_Str *str)
{
	if (!str) {
		return EMPTY_STR;
	}
	return str->c;
}

Weft_Str *str_printf(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	Weft_Str *str = str_printf_v(fmt, args);
	va_end(args);

	return str;
}

static size_t get_printf_len(const char *fmt, va_list args_src)
{
	va_list args;
	va_copy(args, args_src);
	size_t len = vsnprintf(NULL, 0, fmt, args) + 1;
	va_end(args);

	return len;
}

Weft_Str *str_printf_v(const char *fmt, va_list args)
{
	size_t len = get_printf_len(fmt, args);
	Weft_Str *str = new_str(len);
	vsnprintf(str->c, len, fmt, args);

	return str;
}

void str_mark(Weft_Str *str)
{
	gc_mark(str);
}

static size_t get_print_span(const char *src)
{
	size_t span = 0;
	while (char_is_printable(src[span])) {
		span++;
	}
	return span;
}

Weft_Str *str_stringify(const Weft_Str *str)
{
	Weft_Str *output = NULL;
	const char *src = str->c;

	while (*src) {
		size_t span = get_print_span(src);
		if (span) {
			output = str_join(output, new_str_from_n(src, span));
			src += span;
		} else {
			output = str_join(output, char_stringify_esc(*src));
			src++;
		}
	}

	if (!output) {
		return new_str_from_n(str_and_len_of("\"\""));
	}
	return str_printf("\"%.*s\"", output->len, output->c);
}

bool str_is_equal(const Weft_Str *left, const Weft_Str *right)
{
	if (left == right) {
		return true;
	} else if (!left || !right || left->len != right->len) {
		return false;
	}
	return !strncmp(left->c, right->c, left->len);
}

Weft_Str *str_join(Weft_Str *left, Weft_Str *right)
{
	if (!left || !left->len) {
		return right;
	} else if (!right || !right->len) {
		return left;
	}
	return str_printf("%.*s%.*s", left->len, left->c, right->len, right->c);
}
