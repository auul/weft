#ifndef WEFT_STR_H
#define WEFT_STR_H

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

// Forward Declarations

typedef struct weft_str Weft_Str;

// Data Types

struct weft_str {
	size_t len;
	char c[];
};

// Functions

Weft_Str *new_str(size_t len);
Weft_Str *new_str_from(const char *src);
Weft_Str *new_str_from_n(const char *src, size_t len);
size_t str_get_len(const Weft_Str *str);
const char *str_get_c(const Weft_Str *str);
Weft_Str *str_printf(const char *fmt, ...);
Weft_Str *str_printf_v(const char *fmt, va_list args);
Weft_Str *str_stringify(const Weft_Str *str);
void str_mark(Weft_Str *str);
bool str_is_equal(const Weft_Str *left, const Weft_Str *right);
Weft_Str *str_join(Weft_Str *left, Weft_Str *right);

#endif
