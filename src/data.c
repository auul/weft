#include "data.h"
#include "bool.h"
#include "fn.h"
#include "list.h"
#include "num.h"
#include "shuffle.h"

#include <stdio.h>

Weft_Data data_tag_bool(bool bnum)
{
	Weft_Data data = {
		.type = WEFT_DATA_BOOL,
		.bnum = bnum,
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

Weft_Data data_tag_ptr(Weft_DataType type, void *ptr)
{
	Weft_Data data = {
		.type = type,
		.ptr = ptr,
	};
	return data;
}

Weft_DataType data_type(const Weft_Data data)
{
	return data.type;
}

bool data_bool(const Weft_Data data)
{
	return data.bnum;
}

double data_num(const Weft_Data data)
{
	return data.num;
}

void *data_ptr(Weft_Data data)
{
	return data.ptr;
}

const void *data_const_ptr(const Weft_Data data)
{
	return data.ptr;
}

void data_print(const Weft_Data data)
{
	switch (data_type(data)) {
	case WEFT_DATA_BOOL:
		return bool_print(data_bool(data));
	case WEFT_DATA_NUM:
		return num_print(data_num(data));
	case WEFT_DATA_LIST:
		return list_print(data_const_ptr(data));
	case WEFT_DATA_FN:
		return fn_print(data_const_ptr(data));
	case WEFT_DATA_SHUFFLE:
		return shuffle_print(data_const_ptr(data));
	default:
		printf("%u:%p", data_type(data), data_const_ptr(data));
		return;
	}
}

void data_mark(Weft_Data data)
{
	switch (data_type(data)) {
	case WEFT_DATA_LIST:
		return list_mark(data_ptr(data));
	case WEFT_DATA_FN:
		return fn_mark(data_ptr(data));
	case WEFT_DATA_SHUFFLE:
		return shuffle_mark(data_ptr(data));
	default:
		return;
	}
}

bool data_equal(const Weft_Data left, const Weft_Data right)
{
	if (data_type(left) != data_type(right)) {
		return false;
	}

	switch (data_type(left)) {
	case WEFT_DATA_BOOL:
		return bool_equal(data_bool(left), data_bool(right));
	case WEFT_DATA_NUM:
		return num_equal(data_num(left), data_num(right));
	case WEFT_DATA_LIST:
		return list_equal(data_ptr(left), data_ptr(right));
	case WEFT_DATA_FN:
		return fn_equal(data_ptr(left), data_ptr(right));
	case WEFT_DATA_SHUFFLE:
		return shuffle_equal(data_ptr(left), data_ptr(right));
	default:
		return data_ptr(left) == data_ptr(right);
	}
}
