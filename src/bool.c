#include "bool.h"
#include "str.h"

#define str_and_len_of(str) str, sizeof(str) - 1

Weft_Str *bool_stringify(bool b)
{
	if (b) {
		return new_str_from_n(str_and_len_of("true"));
	} else {
		return new_str_from_n(str_and_len_of("false"));
	}
}

bool bool_is_equal(bool left, bool right)
{
	return (left && right) || (!left && !right);
}
