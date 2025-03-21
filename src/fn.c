#include "fn.h"
#include "gc.h"
#include "list.h"

#include <string.h>

static Weft_Fn *new_fn(const Weft_Word name, uintptr_t link)
{
	Weft_Fn *fn = gc_alloc(sizeof(Weft_Fn) + name.len + 1);
	fn->link = link;
	memcpy(fn->name, name.src, name.len);
	fn->name[name.len] = 0;

	return fn;
}

Weft_Fn *new_fn_builtin(const Weft_Word name, bool (*builtin)(Weft_EvalState *))
{
	return new_fn(name, ((uintptr_t)builtin << 1) | 1);
}

Weft_Fn *new_fn_list(const Weft_Word name, Weft_List *list)
{
	return new_fn(name, (uintptr_t)list << 1);
}

static bool is_link_builtin(const uintptr_t link)
{
	return link & 1;
}

static void *get_link_ptr(const uintptr_t link)
{
	return (void *)(link >> 1);
}

bool fn_is_builtin(const Weft_Fn *fn)
{
	return is_link_builtin(fn->link);
}

void *fn_ptr(const Weft_Fn *fn)
{
	return get_link_ptr(fn->link);
}

void fn_print(const Weft_Fn *fn)
{
	printf("%s", fn->name);
}

void fn_mark(const Weft_Fn *fn)
{
	if (gc_mark(fn) || is_link_builtin(fn->link)) {
		return;
	}
	return list_mark(get_link_ptr(fn->link));
}

void fn_set_list(Weft_Fn *fn, Weft_List *list)
{
	fn->link = (uintptr_t)list << 1;
}
