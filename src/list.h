#ifndef WEFT_LIST_H
#define WEFT_LIST_H

// Forward Declarations

typedef struct weft_list Weft_List;

// Local Includes

#include "data.h"

// Data Types

struct weft_list {
	Weft_Data car;
	Weft_List *cdr;
};

// Functions

Weft_List *new_list_node(Weft_Data car, Weft_List *cdr);
Weft_Data list_get_car(Weft_List *list);
Weft_List *list_get_cdr(Weft_List *list);
void list_set_cdr(Weft_List *list, Weft_List *cdr);
void list_mark(Weft_List *list);
Weft_Str *list_stringify(const Weft_List *list);
Weft_Str *list_stringify_bare(const Weft_List *list);
bool list_is_equal(const Weft_List *left, const Weft_List *right);
Weft_List *list_edit(Weft_List **list_p, Weft_List *node);
Weft_List *list_join(Weft_List *left, Weft_List *right);
Weft_List *list_append(Weft_List *list, Weft_Data data);
Weft_Data list_pop(Weft_List **list_p);

#endif
