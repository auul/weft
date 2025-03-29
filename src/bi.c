#include "bi.h"
#include "data.h"
#include "error.h"
#include "eval.h"
#include "fn.h"
#include "list.h"
#include "map.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static bool error_msg(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);

	return false;
}

bool bi_false(Weft_EvalState *W)
{
	eval_push_stack(W, data_tag_bool(false));
	return true;
}

bool bi_true(Weft_EvalState *W)
{
	eval_push_stack(W, data_tag_bool(true));
	return true;
}

bool bi_not(Weft_EvalState *W)
{
	if (eval_get_stack_size(W) < 1
	    || eval_type_mismatch(W, 0, WEFT_DATA_BOOL)) {
		return error_msg("Bool not := Bool");
	}

	Weft_Data a = eval_pop_stack(W);
	eval_push_stack(W, data_tag_bool(!data_get_bool(a)));

	return true;
}

bool bi_or(Weft_EvalState *W)
{
	if (eval_get_stack_size(W) < 2 || eval_type_mismatch(W, 0, WEFT_DATA_BOOL)
	    || eval_type_mismatch(W, 1, WEFT_DATA_BOOL)) {
		return error_msg("Bool Bool or := Bool");
	}

	Weft_Data b = eval_pop_stack(W);
	Weft_Data a = eval_pop_stack(W);

	eval_push_stack(W, data_tag_bool(data_get_bool(a) || data_get_bool(b)));

	return true;
}

bool bi_and(Weft_EvalState *W)
{
	if (eval_get_stack_size(W) < 2 || eval_type_mismatch(W, 0, WEFT_DATA_BOOL)
	    || eval_type_mismatch(W, 1, WEFT_DATA_BOOL)) {
		return error_msg("Bool Bool and := Bool");
	}

	Weft_Data b = eval_pop_stack(W);
	Weft_Data a = eval_pop_stack(W);

	eval_push_stack(W, data_tag_bool(data_get_bool(a) && data_get_bool(b)));

	return true;
}

bool bi_neg(Weft_EvalState *W)
{
	if (eval_get_stack_size(W) < 1 || eval_type_mismatch(W, 0, WEFT_DATA_NUM)) {
		return error_msg("Num neg := Num");
	}

	Weft_Data a = eval_pop_stack(W);
	eval_push_stack(W, data_tag_num(-data_get_num(a)));

	return true;
}

bool bi_add(Weft_EvalState *W)
{
	if (eval_get_stack_size(W) < 2 || eval_type_mismatch(W, 0, WEFT_DATA_NUM)
	    || eval_type_mismatch(W, 1, WEFT_DATA_NUM)) {
		return error_msg("Num Num + := Num");
	}

	Weft_Data b = eval_pop_stack(W);
	Weft_Data a = eval_pop_stack(W);
	eval_push_stack(W, data_tag_num(data_get_num(a) + data_get_num(b)));

	return true;
}

bool bi_subtract(Weft_EvalState *W)
{
	if (eval_get_stack_size(W) < 2 || eval_type_mismatch(W, 0, WEFT_DATA_NUM)
	    || eval_type_mismatch(W, 1, WEFT_DATA_NUM)) {
		return error_msg("Num Num - := Num");
	}

	Weft_Data b = eval_pop_stack(W);
	Weft_Data a = eval_pop_stack(W);
	eval_push_stack(W, data_tag_num(data_get_num(a) - data_get_num(b)));

	return true;
}

bool bi_multiply(Weft_EvalState *W)
{
	if (eval_get_stack_size(W) < 2 || eval_type_mismatch(W, 0, WEFT_DATA_NUM)
	    || eval_type_mismatch(W, 1, WEFT_DATA_NUM)) {
		return error_msg("Num Num * := Num");
	}

	Weft_Data b = eval_pop_stack(W);
	Weft_Data a = eval_pop_stack(W);
	eval_push_stack(W, data_tag_num(data_get_num(a) * data_get_num(b)));

	return true;
}

bool bi_divide(Weft_EvalState *W)
{
	if (eval_get_stack_size(W) < 2 || eval_type_mismatch(W, 0, WEFT_DATA_NUM)
	    || eval_type_mismatch(W, 1, WEFT_DATA_NUM)) {
		return error_msg("Num Num / := Num");
	}

	Weft_Data b = eval_pop_stack(W);
	Weft_Data a = eval_pop_stack(W);
	eval_push_stack(W, data_tag_num(data_get_num(a) / data_get_num(b)));

	return true;
}

bool bi_equal(Weft_EvalState *W)
{
	if (eval_get_stack_size(W) < 2) {
		return error_msg("[A] [B] equal := Bool");
	}

	Weft_Data b = eval_pop_stack(W);
	Weft_Data a = eval_pop_stack(W);
	eval_push_stack(W, data_tag_bool(data_is_equal(a, b)));

	return true;
}

bool bi_head(Weft_EvalState *W)
{
	if (eval_get_stack_size(W) < 1
	    || eval_type_mismatch(W, 0, WEFT_DATA_LIST)) {
		return error_msg("[A ...] head := A");
	}

	Weft_List *list = data_get_ptr(eval_pop_stack(W));
	eval_push_ctrl(W, list_pop(&list));

	return true;
}

bool bi_tail(Weft_EvalState *W)
{
	if (eval_get_stack_size(W) < 1
	    || eval_type_mismatch(W, 0, WEFT_DATA_LIST)) {
		return error_msg("[A ...] tail := [...]");
	}

	Weft_List *list = data_get_ptr(eval_pop_stack(W));
	list_pop(&list);
	eval_push_stack(W, data_tag_ptr(WEFT_DATA_LIST, list));

	return true;
}

bool bi_cons(Weft_EvalState *W)
{
	if (eval_get_stack_size(W) < 2) {
		return error_msg("[A] [B] cons := [[A] B]");
	}

	Weft_Data b = eval_pop_stack(W);
	Weft_Data a = eval_pop_stack(W);

	Weft_List *list;
	if (data_get_type(b) == WEFT_DATA_LIST) {
		list = new_list_node(a, data_get_ptr(b));
	} else {
		list = new_list_node(a, new_list_node(b, NULL));
	}
	eval_push_stack(W, data_tag_ptr(WEFT_DATA_LIST, list));

	return true;
}

bool bi_cat(Weft_EvalState *W)
{
	if (eval_get_stack_size(W) < 2) {
		return error_msg("[A] [B] cat := [A B]");
	}

	Weft_Data b = eval_pop_stack(W);
	Weft_Data a = eval_pop_stack(W);

	Weft_List *list;
	if (data_get_type(a) == WEFT_DATA_LIST) {
		if (data_get_type(b) == WEFT_DATA_LIST) {
			list = list_join(data_get_ptr(a), data_get_ptr(b));
		} else {
			list = list_append(data_get_ptr(a), b);
		}
	} else if (data_get_type(b) == WEFT_DATA_LIST) {
		list = new_list_node(a, data_get_ptr(b));
	} else {
		list = new_list_node(a, new_list_node(b, NULL));
	}
	eval_push_stack(W, data_tag_ptr(WEFT_DATA_LIST, list));

	return true;
}

bool bi_eval(Weft_EvalState *W)
{
	if (eval_get_stack_size(W) < 1) {
		return error_msg("[A] eval := A");
	} else if (eval_type_mismatch(W, 0, WEFT_DATA_LIST)) {
		return true;
	}

	Weft_Data a = eval_pop_stack(W);
	if (!data_get_ptr(a)) {
		return true;
	}
	return eval_list(W, data_get_ptr(a));
}

bool bi_if_else(Weft_EvalState *W)
{
	if (eval_get_stack_size(W) < 3
	    || eval_type_mismatch(W, 2, WEFT_DATA_BOOL)) {
		return error_msg("Bool [A] [B] if-else := A|B");
	}

	Weft_Data c = eval_pop_stack(W);
	Weft_Data b = eval_pop_stack(W);
	Weft_Data a = eval_pop_stack(W);

	if (data_get_bool(a)) {
		return eval_list(W, data_get_ptr(b));
	} else {
		return eval_list(W, data_get_ptr(c));
	}
}

bool bi_dot(Weft_EvalState *W)
{
	if (eval_get_stack_size(W) < 1) {
		return error_msg("[A] . :=");
	}

	Weft_Data a = eval_pop_stack(W);

	data_print(a);
	printf("\n");

	return true;
}

static Weft_Map *
new_bi(Weft_Map *map, const char *name, bool (*builtin)(Weft_EvalState *))
{
	return map_insert(map, new_fn_builtin(name, strlen(name), builtin));
}

Weft_Map *bi_init(void)
{
	Weft_Map *map = NULL;
	map = new_bi(map, "false", bi_false);
	map = new_bi(map, "true", bi_true);
	map = new_bi(map, "not", bi_not);
	map = new_bi(map, "or", bi_or);
	map = new_bi(map, "and", bi_and);
	map = new_bi(map, "neg", bi_neg);
	map = new_bi(map, "+", bi_add);
	map = new_bi(map, "-", bi_subtract);
	map = new_bi(map, "*", bi_multiply);
	map = new_bi(map, "/", bi_divide);
	map = new_bi(map, "=", bi_equal);
	map = new_bi(map, "head", bi_head);
	map = new_bi(map, "tail", bi_tail);
	map = new_bi(map, "cons", bi_cons);
	map = new_bi(map, "cat", bi_cat);
	map = new_bi(map, "eval", bi_eval);
	map = new_bi(map, "if-else", bi_if_else);
	map = new_bi(map, ".", bi_dot);

	return map;
}
