#ifndef WEFT_FN_H
#define WEFT_FN_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Forward Declarations

typedef struct weft_str Weft_Str;
typedef struct weft_list Weft_List;
typedef void *Weft_Builtin;
typedef struct weft_fn Weft_Fn;
typedef uintptr_t Weft_FnLink;

// Data Types

struct weft_fn {
	Weft_FnLink link;
	char name[];
};

// Functions

Weft_Fn *new_fn(const char *name, size_t name_len, Weft_FnLink link);
Weft_Fn *
new_fn_builtin(const char *name, size_t name_len, Weft_Builtin builtin);
Weft_Fn *new_fn_list(const char *name, size_t name_len, Weft_List *list);
bool fn_is_builtin(const Weft_Fn *fn);
const char *fn_get_name(const Weft_Fn *fn);
void *fn_get_ptr(Weft_Fn *fn);
const void *fn_get_const_ptr(const Weft_Fn *fn);
void fn_set_ptr(Weft_Fn *fn, void *ptr);
void fn_mark(Weft_Fn *fn);
Weft_Str *fn_stringify(const Weft_Fn *fn);
Weft_Str *fn_stringify_def(const Weft_Fn *fn);
bool fn_is_equal(const Weft_Fn *left, const Weft_Fn *right);

#endif
