#include "list.h"
#include "data.h"
#include "gc.h"
#include "str.h"

#define str_and_len_of(str) str, sizeof(str) - 1

Weft_List *new_list_node(Weft_Data car, Weft_List *cdr)
{
	Weft_List *node = gc_alloc(sizeof(Weft_List));
	node->car = car;
	node->cdr = cdr;

	return node;
}

Weft_Data list_get_car(Weft_List *list)
{
	return list->car;
}

Weft_List *list_get_cdr(Weft_List *list)
{
	return list->cdr;
}

void list_set_cdr(Weft_List *list, Weft_List *cdr)
{
	list->cdr = cdr;
}

void list_mark(Weft_List *list)
{
	while (!gc_mark(list)) {
		data_mark(list->car);
		list = list->cdr;
	}
}

Weft_Str *list_stringify(const Weft_List *list)
{
	return str_printf("[%s]", str_get_c(list_stringify_bare(list)));
}

Weft_Str *list_stringify_bare(const Weft_List *list)
{
	Weft_Str *output = NULL;
	while (list) {
		output = str_join(output, data_stringify(list->car));
		list = list->cdr;
		if (list) {
			output = str_join(output, new_str_from_n(str_and_len_of(" ")));
		}
	}
	return output;
}

bool list_is_equal(const Weft_List *left, const Weft_List *right)
{
	while (left) {
		if (left == right) {
			return true;
		} else if (!right || !data_is_equal(left->car, right->car)) {
			return false;
		}

		left = left->cdr;
		right = right->cdr;
	}
	return !right;
}

static Weft_List *clone_node(Weft_List *node)
{
	return new_list_node(node->car, node->cdr);
}

Weft_List *list_edit(Weft_List **list_p, Weft_List *node)
{
	Weft_List *list = *list_p;
	if (!list) {
		return NULL;
	}

	list = clone_node(list);
	*list_p = list;

	while (list->cdr != node) {
		list->cdr = clone_node(list->cdr);
		list = list->cdr;
	}

	if (node) {
		list->cdr = clone_node(node);
		return list->cdr;
	}
	return list;
}

Weft_List *list_join(Weft_List *left, Weft_List *right)
{
	if (!left) {
		return right;
	} else if (!right) {
		return left;
	}

	Weft_List *node = list_edit(&left, NULL);
	node->cdr = right;

	return left;
}

Weft_List *list_append(Weft_List *list, Weft_Data data)
{
	return list_join(list, new_list_node(data, NULL));
}

Weft_Data list_pop(Weft_List **list_p)
{
	Weft_List *list = *list_p;
	*list_p = list->cdr;

	return list->car;
}
