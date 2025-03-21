#include "data.h"
#include "fn.h"
#include "list.h"

Weft_Data data_tag_fn(Weft_Fn *fn)
{
	Weft_Data data = {
		.type = WEFT_DATA_FN,
		.fn = fn,
	};
	return data;
}

Weft_Data data_tag_list(Weft_List *list)
{
	Weft_Data data = {
		.type = WEFT_DATA_LIST,
		.list = list,
	};
	return data;
}

Weft_DataType data_type(const Weft_Data data)
{
	return data.type;
}

void *data_ptr(const Weft_Data data)
{
	return (void *)data.ptr;
}

void data_print(const Weft_Data data)
{
	switch (data.type) {
	case WEFT_DATA_FN:
		return fn_print(data.fn);
	case WEFT_DATA_LIST:
		return list_print(data.list);
	default:
		printf("%u:%p", data.type, data.ptr);
		return;
	}
}

void data_mark(const Weft_Data data)
{
	switch (data.type) {
	case WEFT_DATA_FN:
		return fn_mark(data.fn);
	case WEFT_DATA_LIST:
		return list_mark(data.list);
	default:
		return;
	}
}
