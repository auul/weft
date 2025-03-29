#include "fn.h"
#include "gc.h"
#include "list.h"
#include "str.h"

#include <string.h>

Weft_Fn *new_fn(const char *name, size_t name_len, Weft_FnLink link)
{
	Weft_Fn *fn = gc_alloc(sizeof(Weft_Fn) + name_len + 1);
	memcpy(fn->name, name, name_len);
	fn->name[name_len] = 0;
	fn->link = link;

	return fn;
}

static Weft_FnLink new_link(void *ptr)
{
	return (Weft_FnLink)ptr << 1;
}

static Weft_FnLink set_builtin_bit(Weft_FnLink link)
{
	return link | 1;
}

static Weft_FnLink new_builtin_link(Weft_Builtin builtin)
{
	return set_builtin_bit(new_link(builtin));
}

static Weft_FnLink new_list_link(Weft_List *list)
{
	return new_link(list);
}

static bool is_link_builtin(const Weft_FnLink link)
{
	return link & 1;
}

static void *get_link_ptr(Weft_FnLink link)
{
	return (void *)(link >> 1);
}

Weft_Fn *new_fn_builtin(const char *name, size_t name_len, Weft_Builtin builtin)
{
	return new_fn(name, name_len, new_builtin_link(builtin));
}

Weft_Fn *new_fn_list(const char *name, size_t name_len, Weft_List *list)
{
	return new_fn(name, name_len, new_list_link(list));
}

bool fn_is_builtin(const Weft_Fn *fn)
{
	return is_link_builtin(fn->link);
}

const char *fn_get_name(const Weft_Fn *fn)
{
	return fn->name;
}

void *fn_get_ptr(Weft_Fn *fn)
{
	return get_link_ptr(fn->link);
}

const void *fn_get_const_ptr(const Weft_Fn *fn)
{
	return get_link_ptr(fn->link);
}

void fn_set_ptr(Weft_Fn *fn, void *ptr)
{
	if (fn_is_builtin(fn)) {
		fn->link = new_builtin_link(ptr);
	} else {
		fn->link = new_list_link(ptr);
	}
}

void fn_mark(Weft_Fn *fn)
{
	if (gc_mark(fn) || fn_is_builtin(fn)) {
		return;
	} else {
		return list_mark(fn_get_ptr(fn));
	}
}

Weft_Str *fn_stringify(const Weft_Fn *fn)
{
	return new_str_from(fn->name);
}

Weft_Str *fn_stringify_def(const Weft_Fn *fn)
{
	if (fn_is_builtin(fn)) {
		return str_printf("%s: %p", fn->name, fn_get_const_ptr(fn));
	} else {
		return str_join(str_printf("%s: "),
		                list_stringify_bare(fn_get_const_ptr(fn)));
	}
}

bool fn_is_equal(const Weft_Fn *left, const Weft_Fn *right)
{
	if (left == right) {
		return true;
	} else if (left->link == right->link) {
		return true;
	} else if (fn_is_builtin(left) || fn_is_builtin(right)) {
		return false;
	}
	return list_is_equal(fn_get_const_ptr(left), fn_get_const_ptr(right));
}
