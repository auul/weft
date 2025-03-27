#include "debug.h"

#ifndef WEFT_FN_H
#define WEFT_FN_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef WEFT_DEBUG_ALL
#define WEFT_DEBUG_FN
#endif

// Forward Declarations

typedef struct weft_fn Weft_Fn;
typedef struct weft_list Weft_List;
typedef struct weft_eval_state Weft_EvalState;
typedef bool (*Weft_Builtin)(Weft_EvalState *);

// Data Types

struct weft_fn {
	uintptr_t link;
	char name[];
};

// Functions

Weft_Fn *
new_fn_builtin(const char *name, size_t name_len, Weft_Builtin builtin);
Weft_Fn *new_fn_list(const char *name, size_t name_len, Weft_List *list);
const char *fn_name(const Weft_Fn *fn);
bool fn_is_builtin(const Weft_Fn *fn);
void *fn_ptr(Weft_Fn *fn);
const void *fn_const_ptr(const Weft_Fn *fn);
void fn_print(const Weft_Fn *fn);
void fn_print_def(const Weft_Fn *fn);
void fn_mark(Weft_Fn *fn);
bool fn_equal(const Weft_Fn *left, const Weft_Fn *right);
void fn_set_list(Weft_Fn *Fn, Weft_List *list);

#endif
