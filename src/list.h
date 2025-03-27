#include "debug.h"

#ifndef WEFT_LIST_H
#define WEFT_LIST_H

#include <stddef.h>

#ifdef WEFT_DEBUG_ALL
#define WEFT_DEBUG_LIST
#endif

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
Weft_Data list_car(Weft_List *list);
const Weft_Data list_const_car(const Weft_List *list);
Weft_List *list_cdr(Weft_List *list);
const Weft_List *list_const_cdr(const Weft_List *list);
void list_print(const Weft_List *list);
void list_print_bare(const Weft_List *list);
void list_mark(Weft_List *list);
bool list_equal(const Weft_List *left, const Weft_List *right);
Weft_List *list_end(Weft_List *list);
Weft_Data list_pop(Weft_List **list_p);
Weft_Data list_at_index(Weft_List *list, size_t index);
size_t list_len(const Weft_List *list);
Weft_List *list_join(Weft_List *left, Weft_List *right);
Weft_List *list_reverse(Weft_List *list);

#endif
