#include "eval.h"
#include "buf.h"
#include "data.h"
#include "fn.h"
#include "list.h"
#include "shuffle.h"

#include <stdio.h>
#include <stdlib.h>

void eval_init(Weft_EvalState *W)
{
	W->ctrl = NULL;
	W->stack = new_buf(sizeof(Weft_Data));
	W->nest = new_buf(sizeof(Weft_List *));
}

static Weft_Buf *exit_buf(Weft_Buf *buf)
{
	if (buf) {
		free(buf);
	}
	return NULL;
}

void eval_exit(Weft_EvalState *W)
{
	W->ctrl = NULL;
	W->stack = exit_buf(W->stack);
	W->nest = exit_buf(W->nest);
}

static void mark_stack(Weft_Buf *stack)
{
	Weft_Data *data = buf_get_raw(stack);
	size_t len = buf_get_at(stack) / sizeof(Weft_Data);
	for (size_t i = 0; i < len; i++) {
		data_mark(data[i]);
	}
}

static void mark_nest(Weft_Buf *nest)
{
	Weft_List **list = buf_get_raw(nest);
	size_t len = buf_get_at(nest) / sizeof(Weft_List *);
	for (size_t i = 0; i < len; i++) {
		list_mark(list[i]);
	}
}

void eval_mark(Weft_EvalState *W)
{
	list_mark(W->ctrl);
	mark_stack(W->stack);
	mark_nest(W->nest);
}

size_t eval_get_stack_size(const Weft_EvalState *W)
{
	return buf_get_at(W->stack) / sizeof(Weft_Data);
}

bool eval_type_mismatch(Weft_EvalState *W, size_t index, Weft_DataType type)
{
	Weft_Data *data = buf_peek(W->stack, (index + 1) * sizeof(Weft_Data));
	return data_get_type(*data) != type;
}

void eval_push_stack(Weft_EvalState *W, Weft_Data data)
{
	buf_push(&W->stack, &data, sizeof(Weft_Data));
}

Weft_Data eval_pop_stack(Weft_EvalState *W)
{
	Weft_Data data;
	buf_pop(&data, &W->stack, sizeof(Weft_Data));
	return data;
}

Weft_Data eval_peek_stack(Weft_EvalState *W, size_t index)
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

	buf_push(&W->nest, &W->ctrl, sizeof(Weft_List *));
	W->ctrl = list;

	return true;
}

static bool eval_fn(Weft_EvalState *W, Weft_Fn *fn)
{
	if (fn_is_builtin(fn)) {
		bool (*builtin)(Weft_EvalState *) = fn_get_ptr(fn);
		return builtin(W);
	}
	return eval_list(W, fn_get_ptr(fn));
}

static bool eval_shuffle(Weft_EvalState *W, Weft_Shuffle *shuffle)
{
	if (eval_get_stack_size(W) < shuffle_get_in_count(shuffle)) {
		// TODO error
		return false;
	}

	unsigned in_count = shuffle_get_in_count(shuffle);
	if (!shuffle_get_out_count(shuffle)) {
		while (in_count) {
			eval_pop_stack(W);
			in_count--;
		}
		return true;
	}
	unsigned out_count = shuffle_get_out_count(shuffle);

	Weft_Buf *out = new_buf(sizeof(Weft_Data));
	for (unsigned i = 0; i < out_count; i++) {
		unsigned index = in_count - shuffle_get_out_at(shuffle, i) - 1;
		Weft_Data data = eval_peek_stack(W, index);
		buf_push(&out, &data, sizeof(Weft_Data));
	}

	while (in_count) {
		eval_pop_stack(W);
		in_count--;
	}

	buf_push(&W->stack, buf_get_raw(out), buf_get_at(out));
	free(out);

	return true;
}

bool eval(Weft_EvalState *W, Weft_List *ctrl)
{
	W->ctrl = ctrl;

	do {
		while (W->ctrl) {
			Weft_Data data = list_pop(&W->ctrl);
			switch (data_get_type(data)) {
			case WEFT_DATA_FN:
				if (!eval_fn(W, data_get_ptr(data))) {
					return false;
				}
				break;
			case WEFT_DATA_SHUFFLE:
				if (!eval_shuffle(W, data_get_ptr(data))) {
					return false;
				}
				break;
			default:
				eval_push_stack(W, data);
				break;
			}
		}

		while (!W->ctrl && buf_get_at(W->nest)) {
			W->ctrl = buf_pop_ptr(&W->nest);
		}
	} while (W->ctrl);

	if (buf_get_at(W->stack)) {
		Weft_Data *stack = buf_get_raw(W->stack);
		size_t len = buf_get_at(W->stack) / sizeof(Weft_Data);
		for (size_t i = 0; i < len; i++) {
			data_print(stack[i]);
			if (i < len - 1) {
				printf(" ");
			}
		}
		printf("\n");
	}

	return true;
}
