#include "eval.h"
#include "buf.h"
#include "data.h"
#include "fn.h"
#include "list.h"

#include <stdio.h>
#include <stdlib.h>

void eval_init(Weft_EvalState *W)
{
	W->stack = NULL;
	W->ctrl = NULL;
	W->nest = new_buf(WEFT_EVAL_INIT_NEST_CAP * sizeof(Weft_List *));
}

void eval_exit(Weft_EvalState *W)
{
	W->stack = NULL;
	W->ctrl = NULL;

	if (W->nest) {
		free(W->nest);
		W->nest = NULL;
	}
}

bool eval_list(Weft_EvalState *W, Weft_List *list)
{
	if (!list) {
		return true;
	}

	W->nest = buf_push(W->nest, &W->ctrl, sizeof(Weft_List *));
	W->ctrl = list;

	return true;
}

bool eval_fn(Weft_EvalState *W, Weft_Fn *fn)
{
	if (fn_is_builtin(fn)) {
		bool (*builtin)(Weft_EvalState *) = fn_ptr(fn);
		return builtin(W);
	} else if (!fn_ptr(fn)) {
		// TODO debugging
		W->stack = new_list_node(data_tag_fn(fn), W->stack);
		return true;
	}
	return eval_list(W, fn_ptr(fn));
}

bool eval(Weft_EvalState *W, Weft_List *ctrl)
{
	W->ctrl = ctrl;

	do {
		while (W->ctrl) {
			Weft_Data data = list_pop(&W->ctrl);
			switch (data_type(data)) {
			case WEFT_DATA_FN:
				if (!eval_fn(W, data_ptr(data))) {
					return false;
				}
				break;
			default:
				W->stack = new_list_node(data, W->stack);
				break;
			}
		}

		while (buf_get_at(W->nest) && !W->ctrl) {
			buf_pop(&W->ctrl, W->nest, sizeof(Weft_List *));
		}
	} while (W->ctrl);

	list_print(W->stack);
	printf("\n");

	return true;
}
