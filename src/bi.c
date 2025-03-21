#include "bi.h"
#include "data.h"
#include "error.h"
#include "eval.h"
#include "fn.h"
#include "list.h"
#include "map.h"
#include "word.h"

#include <stddef.h>
#include <string.h>

bool bi_dup(Weft_EvalState *W)
{
	if (!W->stack) {
		return error_msg("[X] dup := [X] [X]");
	}

	Weft_Data a = list_pop(&W->stack);

	W->stack = new_list_node(a, new_list_node(a, W->stack));

	return true;
}

bool bi_eval(Weft_EvalState *W)
{
	if (!W->stack || data_type(W->stack->car) != WEFT_DATA_LIST) {
		return error_msg("[X] eval := X");
	}

	Weft_Data a = list_pop(&W->stack);

	return eval_list(W, data_ptr(a));
}

static Weft_Map *
insert_bi(Weft_Map *map, const char *src, bool (*builtin)(Weft_EvalState *))
{
	Weft_Word word = {
		.src = src,
		.len = strlen(src),
	};
	return map_insert(map, new_fn_builtin(word, builtin));
}

Weft_Map *bi_init(void)
{
	Weft_Map *map = NULL;
	map = insert_bi(map, "dup", bi_dup);
	map = insert_bi(map, "eval", bi_eval);

	return map;
}
