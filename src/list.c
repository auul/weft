#include "list.h"
#include "gc.h"

#include <stdio.h>

Weft_List *new_list_node(Weft_Data car, Weft_List *cdr)
{
	Weft_List *node = gc_alloc(sizeof(Weft_List));
	node->car = car;
	node->cdr = cdr;

	return node;
}

Weft_Data list_car(Weft_List *list)
{
	return list->car;
}

const Weft_Data list_const_car(const Weft_List *list)
{
	return list->car;
}

Weft_List *list_cdr(Weft_List *list)
{
	return list->cdr;
}

const Weft_List *list_const_cdr(const Weft_List *list)
{
	return list->cdr;
}

void list_print(const Weft_List *list)
{
	printf("[");
	list_print_bare(list);
	printf("]");
}

void list_print_bare(const Weft_List *list)
{
	while (list) {
		data_print(list_const_car(list));
		list = list_const_cdr(list);
		if (list) {
			printf(" ");
		}
	}
}

void list_mark(Weft_List *list)
{
	while (!gc_mark(list)) {
		data_mark(list->car);
		list = list->cdr;
	}
}

bool list_equal(const Weft_List *left, const Weft_List *right)
{
	while (left) {
		if (left == right) {
			return true;
		} else if (!right || !data_equal(left->car, right->car)) {
			return false;
		}

		left = left->cdr;
		right = right->cdr;
	}
	return !right;
}

Weft_List *list_end(Weft_List *list)
{
	if (!list) {
		return NULL;
	}

	while (list->cdr) {
		list = list->cdr;
	}
	return list;
}

Weft_Data list_pop(Weft_List **list_p)
{
	Weft_List *list = *list_p;
	*list_p = list->cdr;

	return list->car;
}

Weft_Data list_at_index(Weft_List *list, size_t index)
{
	while (index) {
		list = list->cdr;
		index--;
	}
	return list->car;
}

size_t list_len(const Weft_List *list)
{
	size_t len = 0;
	while (list) {
		len++;
		list = list->cdr;
	}
	return len;
}

Weft_List *list_join(Weft_List *left, Weft_List *right)
{
	if (!left) {
		return right;
	} else if (!right) {
		return left;
	}

	Weft_List *list = new_list_node(list_pop(&left), NULL);
	Weft_List *node = list;

	while (left) {
		node->cdr = new_list_node(list_pop(&left), NULL);
		node = node->cdr;
	}
	node->cdr = right;

	return list;
}

Weft_List *list_reverse(Weft_List *list)
{
	Weft_List *dest = NULL;
	while (list) {
		dest = new_list_node(list_pop(&list), dest);
	}
	return dest;
}
