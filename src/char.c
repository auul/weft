#include "char.h"
#include "str.h"

#include <ctype.h>
#include <stdint.h>

#define str_and_len_of(str) str, sizeof(str) - 1

static bool is_utf8(unsigned char c)
{
	static const unsigned char UTF8_XBYTE = 128;
	static const unsigned char UTF8_2BYTE = 192;
	static const unsigned char UTF8_3BYTE = 224;
	static const unsigned char UTF8_4BYTE = 240;

	static const unsigned char UTF8_XMASK = 192;
	static const unsigned char UTF8_2MASK = 224;
	static const unsigned char UTF8_3MASK = 240;
	static const unsigned char UTF8_4MASK = 248;

	return ((c & UTF8_XMASK) == UTF8_XBYTE) || ((c & UTF8_2MASK) == UTF8_2BYTE)
	    || ((c & UTF8_3MASK) == UTF8_3BYTE) || ((c & UTF8_4MASK) == UTF8_4BYTE);
}

bool char_is_printable(char c)
{
	if (!c || isspace(c) || !isgraph(c)) {
		if (c == ' ' || is_utf8(c)) {
			return true;
		}
		return false;
	}
	return true;
}

Weft_Str *char_stringify(char c)
{
	if (c == '\'') {
		return new_str_from_n(str_and_len_of("'\\\''"));
	} else if (char_is_printable(c)) {
		return str_printf("'%c'", c);
	} else {
		Weft_Str *str = char_stringify_esc(c);
		return str_printf("'%.*s'", str_get_len(str), str_get_c(str));
	}
}

Weft_Str *char_stringify_esc(char c)
{
	switch (c) {
	case '\a':
		return new_str_from_n(str_and_len_of("\\a"));
	case '\b':
		return new_str_from_n(str_and_len_of("\\b"));
	case '\e':
		return new_str_from_n(str_and_len_of("\\e"));
	case '\f':
		return new_str_from_n(str_and_len_of("\\f"));
	case '\n':
		return new_str_from_n(str_and_len_of("\\n"));
	case '\r':
		return new_str_from_n(str_and_len_of("\\r"));
	case '\t':
		return new_str_from_n(str_and_len_of("\\t"));
	case '\v':
		return new_str_from_n(str_and_len_of("\\v"));
	case '\\':
		return new_str_from_n(str_and_len_of("\\\\"));
	default:
		return str_printf("\\%#2x", c);
	}
}

bool char_is_equal(char left, char right)
{
	return left == right;
}
