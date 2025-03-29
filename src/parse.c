#include "parse.h"
#include "bi.h"
#include "buf.h"
#include "data.h"
#include "error.h"
#include "fn.h"
#include "gc.h"
#include "lex.h"
#include "list.h"
#include "map.h"
#include "shuffle.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define len_of(str) (sizeof(str) - 1)

static Weft_Buf *init_buf(size_t size)
{
	return new_buf(size * WEFT_PARSE_INIT_BUF_CAP);
}

void parse_init(Weft_ParseState *P)
{
	P->path = NULL;
	P->start = NULL;

	P->map = bi_init();
	P->map_stack = init_buf(sizeof(Weft_Map *));
	P->def_stack = init_buf(sizeof(Weft_Fn *));

	P->list = NULL;
	P->node = NULL;
	P->list_stack = new_buf(sizeof(Weft_List *));
	P->nest = 0;

	P->indent = 0;
	P->indent_stack = new_buf(sizeof(size_t));

	P->shuffle_mode = WEFT_PARSE_SHUFFLE_OFF;
	P->shuffle_in = new_buf(sizeof(char *));
	P->shuffle_out = new_buf(sizeof(char *));
}

static Weft_Buf *exit_buf(Weft_Buf *buf)
{
	if (buf) {
		free(buf);
	}
	return NULL;
}

void parse_exit(Weft_ParseState *P)
{
	P->path = NULL;
	P->start = NULL;

	P->map = NULL;
	P->map_stack = exit_buf(P->map_stack);
	P->def_stack = exit_buf(P->def_stack);

	P->list = NULL;
	P->node = NULL;
	P->list_stack = exit_buf(P->list_stack);
	P->nest = 0;

	P->indent = 0;
	P->indent_stack = exit_buf(P->indent_stack);

	P->shuffle_mode = WEFT_PARSE_SHUFFLE_OFF;
	P->shuffle_in = exit_buf(P->shuffle_in);
	P->shuffle_out = exit_buf(P->shuffle_out);
}

static Weft_Data inline_fn_if_possible(Weft_Data data)
{
	while (true) {
		if (data_get_type(data) != WEFT_DATA_FN) {
			return data;
		}

		Weft_Fn *fn = data_get_ptr(data);
		if (!fn) {
			return data_nil();
		} else if (fn_is_builtin(fn) || !fn_get_ptr(fn)) {
			return data;
		}

		Weft_List *list = fn_get_ptr(fn);
		if (list_get_cdr(list)) {
			return data;
		}
		data = list_get_car(list);
	}
}

static void output_data(Weft_ParseState *P, Weft_Data data)
{
	data = inline_fn_if_possible(data);
	if (data_get_type(data) == WEFT_DATA_NIL) {
		return;
	}

	if (P->list) {
		list_set_cdr(P->node, new_list_node(data, NULL));
		P->node = list_get_cdr(P->node);
	} else {
		P->list = new_list_node(data, NULL);
		P->node = P->list;
	}
}

static char *new_shuffle_word(const char *word, size_t len)
{
	char *token = gc_alloc(len + 1);
	memcpy(token, word, len);
	token[len] = 0;

	return token;
}

static void
handle_shuffle_in(Weft_ParseState *P, const char *word, size_t word_len)
{
	if (buf_get_at(P->shuffle_in) / sizeof(char *) >= UCHAR_MAX) {
		error_parse(P->path,
		            P->start,
		            word,
		            word_len,
		            "Shuffle diagram can contain a maximum number of %u values "
		            "on each side",
		            UCHAR_MAX);
		return;
	}

	char *token = new_shuffle_word(word, word_len);
	buf_push(&P->shuffle_in, &token, sizeof(char *));
}

static bool
is_shuffle_word_match(const char *target, const char *word, size_t word_len)
{
	for (size_t i = 0; i < word_len; i++) {
		if (target[i] != word[i]) {
			return false;
		}
	}

	if (target[word_len]) {
		return false;
	}
	return true;
}

static int
get_shuffle_index(Weft_ParseState *P, const char *word, size_t word_len)
{
	char **name_p = buf_get_raw(P->shuffle_in);
	int len = buf_get_at(P->shuffle_in) / sizeof(char *);

	for (int i = 0; i < len; i++) {
		if (is_shuffle_word_match(name_p[i], word, word_len)) {
			return i;
		}
	}
	return -1;
}

static void
handle_shuffle_out(Weft_ParseState *P, const char *word, size_t word_len)
{
	if ((buf_get_at(P->shuffle_out) / sizeof(uint8_t)) >= UCHAR_MAX) {
		error_parse(P->path,
		            P->start,
		            word,
		            word_len,
		            "Shuffle diagram can contain a maximum number of %u values "
		            "on each side",
		            UCHAR_MAX);
		return;
	}

	int index = get_shuffle_index(P, word, word_len);
	if (index < 0) {
		error_parse(P->path,
		            P->start,
		            word,
		            word_len,
		            "%.*s not found on left side of shuffle diagram",
		            word_len,
		            word);
		return;
	}

	unsigned char byte = index;
	buf_push(&P->shuffle_out, &byte, sizeof(unsigned char));
}

static void handle_word(Weft_ParseState *P, const char *word, size_t word_len)
{
	switch (P->shuffle_mode) {
	case WEFT_PARSE_SHUFFLE_IN:
		return handle_shuffle_in(P, word, word_len);
	case WEFT_PARSE_SHUFFLE_OUT:
		return handle_shuffle_out(P, word, word_len);
	default:
		break;
	}

	Weft_Fn *fn = map_lookup(P->map, word, word_len);
	if (fn) {
		output_data(P, data_tag_ptr(WEFT_DATA_FN, fn));
	} else {
		error_parse(P->path,
		            P->start,
		            word,
		            word_len,
		            "%.*s is undefined",
		            word_len,
		            word);
	}
}

static void handle_def(Weft_ParseState *P, const char *word, size_t word_len)
{
	if (P->shuffle_mode || P->nest) {
		error_parse(P->path,
		            P->start,
		            word,
		            word_len,
		            "Function definitions must be top level");
		return;
	}

	Weft_Fn *fn = new_fn_list(word, word_len, NULL);

	P->map = map_insert(P->map, fn);
	buf_push(&P->map_stack, &P->map, sizeof(Weft_Map *));
	buf_push(&P->def_stack, &fn, sizeof(Weft_Fn *));

	buf_push(&P->list_stack, &P->list, sizeof(Weft_List *));
	P->list = NULL;
	P->node = NULL;

	buf_push(&P->indent_stack, &P->indent, sizeof(size_t));
}

static void handle_dedent(Weft_ParseState *P)
{
	buf_pop(&P->map, &P->map_stack, sizeof(Weft_Map *));

	Weft_Fn *fn;
	buf_pop(&fn, &P->def_stack, sizeof(Weft_Fn *));
	fn_set_ptr(fn, P->list);

	buf_pop(&P->list, &P->list_stack, sizeof(Weft_List *));
	P->node = list_edit(&P->list, NULL);

	buf_drop(&P->indent_stack, sizeof(size_t));
}

static bool is_dedent(const Weft_ParseState *P, size_t indent)
{
	if (!buf_get_at(P->indent_stack)) {
		return false;
	}
	return buf_peek_size(P->indent_stack, 0) >= indent;
}

static void handle_indent(Weft_ParseState *P, size_t indent)
{
	if (P->shuffle_mode) {
		return;
	}

	while (is_dedent(P, indent)) {
		handle_dedent(P);
	}
	P->indent = indent;
}

static void handle_list_open(Weft_ParseState *P, const char *src)
{
	if (P->shuffle_mode) {
		return error_parse(P->path,
		                   P->start,
		                   src,
		                   len_of("["),
		                   "Cannot define a list within a shuffle diagram");
	}

	buf_push(&P->list_stack, &P->list, sizeof(Weft_List *));
	P->list = NULL;
	P->node = NULL;
	P->nest++;
}

static void handle_list_close(Weft_ParseState *P, const char *src)
{
	if (P->shuffle_mode) {
		error_parse(P->path,
		            P->start,
		            src,
		            len_of("]"),
		            "Cannot define a list within a shuffle diagram");
		return;
	} else if (!P->nest) {
		error_parse(P->path, P->start, src, len_of("]"), "Unmatched ']'");
		return;
	}

	Weft_List *list = P->list;
	buf_pop(&P->list, &P->list_stack, sizeof(Weft_List *));
	P->node = list_edit(&P->list, NULL);

	output_data(P, data_tag_ptr(WEFT_DATA_LIST, list));
	P->nest--;
}

static void handle_shuffle_open(Weft_ParseState *P, const char *src)
{
	if (P->shuffle_mode) {
		error_parse(P->path,
		            P->start,
		            src,
		            len_of("{"),
		            "Cannot nest shuffle diagrams");
		return;
	}
	P->shuffle_mode = WEFT_PARSE_SHUFFLE_IN;
}

static void handle_shuffle_close(Weft_ParseState *P, const char *src)
{
	switch (P->shuffle_mode) {
	case WEFT_PARSE_SHUFFLE_OFF:
		error_parse(P->path, P->start, src, len_of("}"), "Unmatched '}'");
		return;
	case WEFT_PARSE_SHUFFLE_IN:
		error_parse(P->path,
		            P->start,
		            src,
		            len_of("}"),
		            "Shuffle diagram must contain a left and right side "
		            "separated by --");
		break;
	default:
		break;
	}
	P->shuffle_mode = WEFT_PARSE_SHUFFLE_OFF;

	Weft_Shuffle *shuffle =
		new_shuffle_empty(buf_get_at(P->shuffle_in) / sizeof(char *),
	                      buf_get_at(P->shuffle_out) / sizeof(uint8_t));
	uint8_t *out = buf_get_raw(P->shuffle_out);
	for (unsigned i = 0; i < shuffle_get_out_count(shuffle); i++) {
		shuffle_set_out_at(shuffle, i, out[i]);
	}

	buf_clear(&P->shuffle_in);
	buf_clear(&P->shuffle_out);

	output_data(P, data_tag_ptr(WEFT_DATA_SHUFFLE, shuffle));
}

static bool is_shuffle_mid(const Weft_ParseState *P, const char *src)
{
	return P->shuffle_mode == WEFT_PARSE_SHUFFLE_IN && src[0] == '-'
	    && src[1] == '-';
}

Weft_List *parse(Weft_ParseState *P, const char *path, const char *src)
{
	P->path = path;
	P->start = src;

	Weft_Lex lex;
	if (lex_is_line_empty(src)) {
		src += lex_empty(src).len;
	} else {
		lex = lex_indent(src);
		handle_indent(P, lex.len);
		src += lex.len;
	}

	while (*src) {
		lex = lex_token(P->path, P->start, src);
		switch (lex.type) {
		case WEFT_LEX_INDENT:
			handle_indent(P, lex.len - len_of("\n"));
			src += lex.len;
			break;
		case WEFT_LEX_EMPTY:
			src += lex.len;
			break;
		case WEFT_LEX_NUM:
			output_data(P, data_tag_num(lex.num));
			src += lex.len;
			break;
		case WEFT_LEX_CHAR:
			output_data(P, data_tag_char(lex.c));
			src += lex.len;
			break;
		case WEFT_LEX_STR:
			output_data(P, data_tag_ptr(WEFT_DATA_STR, lex.str));
			src += lex.len;
			break;
		case WEFT_LEX_QUOTATION_OPEN:
			handle_list_open(P, lex.src);
			src += lex.len;
			break;
		case WEFT_LEX_QUOTATION_CLOSE:
			handle_list_close(P, lex.src);
			src += lex.len;
			break;
		case WEFT_LEX_SHUFFLE_OPEN:
			handle_shuffle_open(P, lex.src);
			src += lex.len;
			break;
		case WEFT_LEX_SHUFFLE_CLOSE:
			handle_shuffle_close(P, lex.src);
			src += lex.len;
			break;
		case WEFT_LEX_WORD:
			if (is_shuffle_mid(P, lex.src)) {
				P->shuffle_mode = WEFT_PARSE_SHUFFLE_OUT;
				src += len_of("--");
			} else {
				handle_word(P, lex.src, lex.len);
				src += lex.len;
			}
			break;
		case WEFT_LEX_DEF:
			handle_def(P, lex.src, lex.len - len_of(":"));
			src += lex.len;
			break;
		}
		src += lex_empty(src).len;
	}

	if (buf_get_at(P->list_stack)) {
		return NULL;
	}

	Weft_List *list = P->list;
	P->list = NULL;
	P->node = NULL;

	list_mark(list);

	return list;
}
