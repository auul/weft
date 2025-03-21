#ifndef WEFT_FN_H
#define WEFT_FN_H

#include <stdbool.h>
#include <stdint.h>

// Forward Declarations

typedef struct weft_fn Weft_Fn;
typedef struct weft_list Weft_List;
typedef struct weft_eval_state Weft_EvalState;

// Local Includes

#include "word.h"

// Data Types

struct weft_fn {
	uintptr_t link;
	char name[];
};

// Functions

Weft_Fn *new_fn_builtin(const Weft_Word name,
                        bool (*builtin)(Weft_EvalState *));
Weft_Fn *new_fn_list(const Weft_Word name, Weft_List *list);
bool fn_is_builtin(const Weft_Fn *fn);
void *fn_ptr(const Weft_Fn *fn);
void fn_print(const Weft_Fn *fn);
void fn_mark(const Weft_Fn *fn);
void fn_set_list(Weft_Fn *fn, Weft_List *list);

#endif
