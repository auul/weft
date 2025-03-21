#include "parse.h"
#include "bi.h"
#include "buf.h"
#include "error.h"
#include "fn.h"
#include "lex.h"
#include "list.h"
#include "map.h"

#include <stdio.h>
#include <stdlib.h>

static void clear_word(Weft_ParseState *P)
{
	P->word.src = NULL;
	P->word.len = 0;
}

static bool is_word_empty(Weft_ParseState *P)
{
	return P->word.src == NULL;
}

static void init_stack(Weft_Buf **stack_p, size_t stride)
{
	*stack_p = new_buf(WEFT_PARSE_INIT_STACK_CAP * stride);
}

static void exit_stack(Weft_Buf **stack_p)
{
	Weft_Buf *stack = *stack_p;
	if (!stack) {
		return;
	}
	*stack_p = NULL;

	free(stack);
}

void mark_stack(Weft_Buf *stack, size_t stride, void *mark_fn_p)
{
	void (*mark_fn)(void *) = mark_fn_p;
	for (size_t i = 0; i < buf_get_at(stack); i += stride) {
		void **ptr_p = (void **)(buf_get_raw(stack) + i);
		mark_fn(*ptr_p);
	}
}

void parse_init(Weft_ParseState *P)
{
	clear_word(P);
	init_stack(&P->def_stack, sizeof(Weft_Fn *));

	P->indent = 0;
	init_stack(&P->indent_stack, sizeof(size_t));

	P->list = NULL;
	P->node = NULL;
	init_stack(&P->list_stack, sizeof(Weft_List *));
	P->nest = 0;

	P->map = bi_init();
	init_stack(&P->map_stack, sizeof(Weft_Map *));
}

void parse_exit(Weft_ParseState *P)
{
	clear_word(P);
	exit_stack(&P->def_stack);

	P->indent = 0;
	exit_stack(&P->indent_stack);

	P->list = NULL;
	P->node = NULL;
	exit_stack(&P->list_stack);
	P->nest = 0;

	P->map = NULL;
	exit_stack(&P->map_stack);
}

void parse_error(const char *src, size_t at, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	parse_error_v(src, at, fmt, args);
	va_end(args);
}

void parse_error_v(const char *src, size_t at, const char *fmt, va_list args)
{
	error_msg_v(fmt, args);
}

static void append_data(Weft_ParseState *P, Weft_Data data)
{
	if (P->list) {
		P->node->cdr = new_list_node(data, NULL);
		P->node = P->node->cdr;
	} else {
		P->list = new_list_node(data, NULL);
		P->node = P->list;
	}
}

static void flush_word(Weft_ParseState *P)
{
	if (is_word_empty(P)) {
		return;
	}

	Weft_Fn *fn = map_lookup(P->map, P->word);
	if (!fn) {
		// TODO DEBUG
		fn = new_fn_list(P->word, NULL);
	}
	clear_word(P);

	return append_data(P, data_tag_fn(fn));
}

static bool is_dedent(const Weft_ParseState *P, size_t indent)
{
	if (!buf_get_at(P->indent_stack)) {
		return false;
	}

	const size_t *base = buf_peek(P->indent_stack, sizeof(size_t));
	return indent <= *base;
}

static void handle_dedent(Weft_ParseState *P)
{
	flush_word(P);

	size_t indent;
	Weft_Fn *fn;
	Weft_List *list;

	buf_pop(&indent, P->indent_stack, sizeof(size_t));
	buf_pop(&fn, P->def_stack, sizeof(Weft_Fn *));

	list = P->list;
	buf_pop(&P->list, P->list_stack, sizeof(Weft_List *));
	P->node = list_end(P->list);
	fn_set_list(fn, list);

	buf_pop(&P->map, P->map_stack, sizeof(Weft_Map *));
}

static void handle_indent(Weft_ParseState *P, size_t indent)
{
	while (is_dedent(P, indent)) {
		handle_dedent(P);
	}
	P->indent = indent;
}

static void handle_word(Weft_ParseState *P, const Weft_Word word)
{
	flush_word(P);
	P->word = word;
}

static void handle_open_list(Weft_ParseState *P)
{
	P->list_stack = buf_push(P->list_stack, &P->list, sizeof(Weft_List *));
	P->list = NULL;
	P->node = NULL;
	P->nest += 1;
}

static void handle_close_list(Weft_ParseState *P)
{
	flush_word(P);

	Weft_List *list = P->list;
	buf_pop(&P->list, P->list_stack, sizeof(Weft_List *));
	P->nest -= 1;

	if (P->nest) {
		P->node = list_end(P->list);
		P->node->cdr = new_list_node(data_tag_list(list), NULL);
		P->node = P->node->cdr;
	} else {
		P->list = new_list_node(data_tag_list(list), NULL);
		P->node = P->list;
	}
}

static void handle_def(Weft_ParseState *P)
{
	Weft_Fn *fn = map_touch(&P->map, P->word);
	P->def_stack = buf_push(P->def_stack, &fn, sizeof(Weft_Fn *));
	clear_word(P);

	P->indent_stack = buf_push(P->indent_stack, &P->indent, sizeof(size_t));
	P->map_stack = buf_push(P->map_stack, &P->map, sizeof(Weft_Map *));

	P->list_stack = buf_push(P->list_stack, &P->list, sizeof(Weft_List *));
	P->list = NULL;
	P->node = NULL;
}

static void handle_token(Weft_ParseState *P, const Weft_LexData token)
{
	switch (lex_type(token)) {
	case WEFT_LEX_EMPTY:
		break;
	case WEFT_LEX_INDENT:
		if (lex_len(token) == 0) {
			return handle_indent(P, 0);
		}
		return handle_indent(P, lex_len(token) - 1);
	case WEFT_LEX_WORD:
		return handle_word(P, lex_data_to_word(token));
	case WEFT_LEX_OPEN_LIST:
		return handle_open_list(P);
	case WEFT_LEX_CLOSE_LIST:
		if (!P->nest) {
			return parse_error(P->src, token.src - P->src, "Unmatched ]");
		}
		return handle_close_list(P);
	case WEFT_LEX_DEFINE:
		if (is_word_empty(P)) {
			return parse_error(P->src,
			                   token.src - P->src,
			                   "Stray : outside of function definition");
		} else if (P->nest) {
			return parse_error(P->src,
			                   P->word.src - P->src,
			                   "Function definition inside of list literal");
		}
		return handle_def(P);
	}
}

static Weft_List *flush_state(Weft_ParseState *P)
{
	Weft_List *list = P->list;
	P->list = NULL;
	P->node = NULL;
	return list;
}

Weft_List *parse(Weft_ParseState *P, const char *src)
{
	P->src = src;
	P->node = list_end(P->list);

	Weft_LexData token;
	if (lex_is_line_empty(src)) {
		token = lex_empty(src);
		src += lex_len(token);
	} else {
		token = lex_indent(src);
		handle_token(P, token);
		src += lex_len(token);
	}

	while (*src) {
		token = lex_token(src);
		handle_token(P, token);
		src += lex_len(token);

		token = lex_empty(src);
		src += lex_len(token);
	}
	error_flush();

	if (!buf_get_at(P->list_stack)) {
		return flush_state(P);
	}

	return NULL;
}
