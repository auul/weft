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

void list_print(const Weft_List *list)
{
	printf("[");
	list_print_bare(list);
	printf("]");
}

void list_print_bare(const Weft_List *list)
{
	while (list) {
		data_print(list->car);
		list = list->cdr;
		if (list) {
			printf(" ");
		}
	}
}

void list_mark(const Weft_List *list)
{
	while (!gc_mark(list)) {
		data_mark(list->car);
		list = list->cdr;
	}
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
