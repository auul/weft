#include "fn.h"
#include "gc.h"
#include "list.h"

#include <stdio.h>
#include <string.h>

static Weft_Fn *new_fn(const char *name, size_t name_len, uintptr_t link)
{
	Weft_Fn *fn = gc_alloc(sizeof(Weft_Fn) + name_len + 1);
	memcpy(fn->name, name, name_len);
	fn->name[name_len] = 0;
	fn->link = link;

	return fn;
}

Weft_Fn *new_fn_builtin(const char *name, size_t name_len, Weft_Builtin builtin)
{
	return new_fn(name, name_len, ((uintptr_t)builtin << 1) | 1);
}

Weft_Fn *new_fn_list(const char *name, size_t name_len, Weft_List *list)
{
	return new_fn(name, name_len, (uintptr_t)list << 1);
}

const char *fn_name(const Weft_Fn *fn)
{
	return fn->name;
}

bool fn_is_builtin(const Weft_Fn *fn)
{
	return fn->link & 1;
}

void *fn_ptr(Weft_Fn *fn)
{
	return (void *)(fn->link >> 1);
}

const void *fn_const_ptr(const Weft_Fn *fn)
{
	return (const void *)(fn->link >> 1);
}

void fn_print(const Weft_Fn *fn)
{
	printf("%s", fn->name);
}

void fn_print_def(const Weft_Fn *fn)
{
	printf("%s: ", fn->name);
	if (fn_is_builtin(fn)) {
		printf("%p", fn_const_ptr(fn));
	} else {
		list_print_bare(fn_const_ptr(fn));
	}
}

void fn_mark(Weft_Fn *fn)
{
	if (gc_mark(fn)) {
		return;
	} else if (fn_is_builtin(fn)) {
		return;
	}
	return list_mark(fn_ptr(fn));
}

bool fn_equal(const Weft_Fn *left, const Weft_Fn *right)
{
	if (left == right || left->link == right->link) {
		return true;
	} else if (fn_is_builtin(left) || fn_is_builtin(right)) {
		return false;
	}
	return list_equal(fn_const_ptr(left), fn_const_ptr(right));
}

void fn_set_list(Weft_Fn *fn, Weft_List *list)
{
	fn->link = (uintptr_t)list << 1;
}
