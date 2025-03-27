#include "eval.h"
#include "buf.h"
#include "data.h"
#include "error.h"
#include "fn.h"
#include "gc.h"
#include "list.h"
#include "shuffle.h"

#include <stdio.h>
#include <stdlib.h>

#ifdef WEFT_DEBUG_EVAL

void eval_debug(const Weft_EvalState *W)
{
	printf("\nCtrl: ");
	list_print_bare(W->ctrl);

	printf("\nStack: ");
	Weft_Data *stack = buf_raw(W->stack);
	size_t len = buf_at(W->stack) / sizeof(Weft_Data);
	for (size_t i = 0; i < len; i++) {
		data_print(stack[i]);
		if (i < len - 1) {
			printf(" ");
		}
	}

	printf("\nNest: ");
	Weft_List **nest = buf_raw(W->nest);
	len = buf_at(W->nest) / sizeof(Weft_List *);
	for (size_t i = 0; i < len; i++) {
		list_print(nest[i]);
		if (i < len - 1) {
			printf(" ");
		}
	}

	printf("\n");
}
#endif

void eval_init(Weft_EvalState *W)
{
	W->ctrl = NULL;
	W->stack = new_buf(sizeof(Weft_Data));
	W->nest = new_buf(sizeof(Weft_List *));
}

void eval_exit(Weft_EvalState *W)
{
	W->ctrl = NULL;

	if (W->stack) {
		free(W->stack);
		W->stack = NULL;
	}

	if (W->nest) {
		free(W->nest);
		W->nest = NULL;
	}
}

void eval_mark(Weft_EvalState *W)
{
	list_mark(W->ctrl);

	Weft_Data *stack = buf_raw(W->stack);
	size_t len = buf_at(W->stack) / sizeof(Weft_Data);
	for (size_t i = 0; i < len; i++) {
		data_mark(stack[i]);
	}

	Weft_List **nest = buf_raw(W->nest);
	len = buf_at(W->nest) / sizeof(Weft_List *);
	for (size_t i = 0; i < len; i++) {
		list_mark(nest[i]);
	}
}

size_t eval_stack_size(const Weft_EvalState *W)
{
	return buf_at(W->stack) / sizeof(Weft_Data);
}

bool eval_type_mismatch(Weft_EvalState *W, size_t index, Weft_DataType type)
{
	Weft_Data *data = buf_peek(W->stack, (index + 1) * sizeof(Weft_Data));
	return data_type(*data) != type;
}

void eval_push_stack(Weft_EvalState *W, Weft_Data data)
{
	W->stack = buf_push(W->stack, &data, sizeof(Weft_Data));
}

Weft_Data eval_pop_stack(Weft_EvalState *W)
{
	Weft_Data data;
	W->stack = buf_pop(W->stack, &data, sizeof(Weft_Data));
	return data;
}

Weft_Data eval_at_stack_index(Weft_EvalState *W, size_t index)
{
	Weft_Data *data = buf_peek(W->stack, (index + 1) * sizeof(Weft_Data));
	return *data;
}

void eval_push_ctrl(Weft_EvalState *W, Weft_Data data)
{
	W->ctrl = new_list_node(data, W->ctrl);
}

bool eval_list(Weft_EvalState *W, Weft_List *list)
{
	if (!list) {
		return true;
	} else if (!W->ctrl) {
		W->ctrl = list;
		return true;
	}

	W->nest = buf_push(W->nest, &W->ctrl, sizeof(Weft_List *));
	W->ctrl = list;

	return true;
}

static bool eval_fn(Weft_EvalState *W, Weft_Fn *fn)
{
	if (fn_is_builtin(fn)) {
		bool (*builtin)(Weft_EvalState *) = fn_ptr(fn);
		return builtin(W);
	}
	return eval_list(W, fn_ptr(fn));
}

static bool eval_shuffle(Weft_EvalState *W, Weft_Shuffle *shuffle)
{
	if (eval_stack_size(W) < shuffle_in_count(shuffle)) {
		return error_msg("Shuffle lambda expected stack depth %u, but stack "
		                 "depth is only %u",
		                 shuffle_in_count(shuffle),
		                 eval_stack_size(W));
	}

	unsigned in_count = shuffle_in_count(shuffle);
	if (!shuffle_out_count(shuffle)) {
		while (in_count) {
			eval_pop_stack(W);
			in_count--;
		}
		return true;
	}

	Weft_Buf *out = new_buf(sizeof(Weft_Data));
	for (unsigned i = 0; i < shuffle_out_count(shuffle); i++) {
		unsigned index = in_count - shuffle_out_value(shuffle, i) - 1;
		Weft_Data data = eval_at_stack_index(W, index);
		out = buf_push(out, &data, sizeof(Weft_Data));
	}

	while (in_count) {
		eval_pop_stack(W);
		in_count--;
	}

	W->stack = buf_push(W->stack, buf_raw(out), buf_at(out));
	free(out);

	return true;
}

bool eval(Weft_EvalState *W, Weft_List *ctrl)
{
	W->ctrl = ctrl;

	do {
		while (W->ctrl) {
#ifdef WEFT_DEBUG_EVAL
			eval_debug(W);
#endif

			Weft_Data data = list_pop(&W->ctrl);
			switch (data_type(data)) {
			case WEFT_DATA_FN:
				if (!eval_fn(W, data_ptr(data))) {
					error_flush();
					return false;
				}
				break;
			case WEFT_DATA_SHUFFLE:
				if (!eval_shuffle(W, data_ptr(data))) {
					error_flush();
					return false;
				}
				break;
			default:
				eval_push_stack(W, data);
				break;
			}
		}

		while (!W->ctrl && buf_at(W->nest)) {
			W->nest = buf_pop(W->nest, &W->ctrl, sizeof(Weft_List *));
		}

		if (gc_is_ready()) {
			eval_mark(W);
			gc_collect();
		}
	} while (W->ctrl);

	error_flush();
#ifdef WEFT_DEBUG_EVAL
	eval_debug(W);
#endif

	if (buf_at(W->stack)) {
		Weft_Data *top = buf_peek(W->stack, sizeof(Weft_Data));
		data_print(*top);
		printf("\n");
	}

	return true;
}
