#include "data.h"
#include "bool.h"
#include "char.h"
#include "fn.h"
#include "list.h"
#include "num.h"
#include "shuffle.h"
#include "str.h"

#include <stdio.h>

#define str_and_len_of(str) str, sizeof(str) - 1

Weft_Data data_nil(void)
{
	return data_tag_ptr(WEFT_DATA_NIL, NULL);
}

Weft_Data data_tag_bool(bool b)
{
	Weft_Data data = {
		.type = WEFT_DATA_BOOL,
		.b = b,
	};
	return data;
}

Weft_Data data_tag_num(double num)
{
	Weft_Data data = {
		.type = WEFT_DATA_NUM,
		.num = num,
	};
	return data;
}

Weft_Data data_tag_char(unsigned char c)
{
	Weft_Data data = {
		.type = WEFT_DATA_CHAR,
		.c = c,
	};
	return data;
}

Weft_Data data_tag_ptr(Weft_DataType type, void *ptr)
{
	Weft_Data data = {
		.type = type,
		.ptr = ptr,
	};
	return data;
}

Weft_DataType data_get_type(const Weft_Data data)
{
	return data.type;
}

bool data_get_bool(const Weft_Data data)
{
	return data.b;
}

double data_get_num(const Weft_Data data)
{
	return data.num;
}

void *data_get_ptr(Weft_Data data)
{
	return data.ptr;
}

Weft_Str *data_stringify(const Weft_Data data)
{
	switch (data.type) {
	case WEFT_DATA_NIL:
		return new_str_from_n(str_and_len_of("nil"));
	case WEFT_DATA_BOOL:
		return bool_stringify(data.b);
	case WEFT_DATA_NUM:
		return num_stringify(data.num);
	case WEFT_DATA_CHAR:
		return char_stringify(data.c);
	case WEFT_DATA_STR:
		return str_stringify(data.str);
	case WEFT_DATA_LIST:
		return list_stringify(data.list);
	case WEFT_DATA_FN:
		return fn_stringify(data.fn);
	case WEFT_DATA_SHUFFLE:
		return shuffle_stringify(data.shuffle);
	default:
		return str_printf("%u:%p", data.type, data.ptr);
	}
}

void data_print(const Weft_Data data)
{
	printf("%s", str_get_c(data_stringify(data)));
}

void data_mark(Weft_Data data)
{
	switch (data.type) {
	case WEFT_DATA_STR:
		return str_mark(data.str);
	case WEFT_DATA_LIST:
		return list_mark(data.list);
	case WEFT_DATA_FN:
		return fn_mark(data.fn);
	case WEFT_DATA_SHUFFLE:
		return shuffle_mark(data.shuffle);
	default:
		return;
	}
}

bool data_is_equal(const Weft_Data left, const Weft_Data right)
{
	if (left.type != right.type) {
		return false;
	}

	switch (left.type) {
	case WEFT_DATA_NIL:
		return true;
	case WEFT_DATA_BOOL:
		return bool_is_equal(left.b, right.b);
	case WEFT_DATA_NUM:
		return num_is_equal(left.num, right.num);
	case WEFT_DATA_CHAR:
		return char_is_equal(left.c, right.c);
	case WEFT_DATA_STR:
		return str_is_equal(left.str, right.str);
	case WEFT_DATA_LIST:
		return list_is_equal(left.list, right.list);
	case WEFT_DATA_FN:
		return fn_is_equal(left.fn, right.fn);
	case WEFT_DATA_SHUFFLE:
		return shuffle_is_equal(left.shuffle, right.shuffle);
	default:
		return left.ptr == right.ptr;
	}
}
