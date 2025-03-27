#include "parse.h"
#include "bi.h"
#include "buf.h"
#include "error.h"
#include "fn.h"
#include "gc.h"
#include "list.h"
#include "map.h"
#include "shuffle.h"

#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#ifdef WEFT_DEBUG_PARSE
#include <stdio.h>
#endif

// Constants

static const char delim_list[] = "#[]{}:";

// Functions

#ifdef WEFT_DEBUG_PARSE
void parse_debug(const Weft_ParseState *P)
{
	printf("\n");
	map_print(P->map);
	printf("\nMap Level: %zu", buf_at(P->map_stack) / sizeof(Weft_Map *));

	if (P->word) {
		printf("\nWord: %.*s", P->word_len, P->word);
	}
	printf("\nDef Level: %zu", buf_at(P->def_stack) / sizeof(Weft_Fn *));
	printf("\nList: ");
	list_print_bare(P->list);

	printf("\n");
}
#endif

void parse_init(Weft_ParseState *P)
{
	P->start = NULL;

	P->map = bi_init();
	P->map_stack = new_buf(sizeof(Weft_Map *));

	P->word = NULL;
	P->word_len = 0;
	P->def_stack = new_buf(sizeof(Weft_Fn *));

	P->list = NULL;
	P->node = NULL;
	P->list_stack = new_buf(sizeof(Weft_List *));
	P->nest = 0;

	P->indent = 0;
	P->indent_stack = new_buf(sizeof(size_t));

	P->shuffle_mode = WEFT_PARSE_SHUFFLE_OFF;
	P->shuffle_in = new_buf(sizeof(char *));
	P->shuffle_out = new_buf(sizeof(uint8_t));
}

void parse_exit(Weft_ParseState *P)
{
	P->start = NULL;

	P->map = NULL;
	if (P->map_stack) {
		free(P->map_stack);
		P->map_stack = NULL;
	}

	P->word = NULL;
	P->word_len = 0;
	if (P->def_stack) {
		free(P->def_stack);
		P->def_stack = NULL;
	}

	P->list = NULL;
	P->node = NULL;
	if (P->list_stack) {
		free(P->list_stack);
		P->list_stack = NULL;
	}
	P->nest = 0;

	P->indent = 0;
	if (P->indent_stack) {
		free(P->indent_stack);
		P->indent_stack = NULL;
	}

	P->shuffle_mode = WEFT_PARSE_SHUFFLE_OFF;
	if (P->shuffle_in) {
		free(P->shuffle_in);
		P->shuffle_in = NULL;
	}
	if (P->shuffle_out) {
		free(P->shuffle_out);
		P->shuffle_out = NULL;
	}
}

void parse_error(
	const Weft_ParseState *P, const char *src, size_t len, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	parse_error_v(P, src, len, fmt, args);
	va_end(args);
}

static const char *
get_line_at(size_t *line_no_p, const Weft_ParseState *P, const char *at)
{
	*line_no_p = 0;
	const char *src = P->start;
	const char *line = src;

	while (src != at) {
		if (*src == '\n') {
			*line_no_p += 1;
			line = src + 1;
		}
		src++;
	}
	return line;
}

static size_t get_line_len(const char *line)
{
	size_t len = 0;
	while (line[len] && line[len] != '\n') {
		len++;
	}
	return len;
}

static const char *skip_space(const char *src)
{
	while (isspace(*src)) {
		src++;
	}
	return src;
}

void parse_error_v(const Weft_ParseState *P,
                   const char *src,
                   size_t len,
                   const char *fmt,
                   va_list args)
{
	size_t line_no;
	const char *line = get_line_at(&line_no, P, src);
	line = skip_space(line);

	error_msg("Line %zu: %.*s\e[91m%.*s\e[0m%.*s",
	          line_no,
	          src - line,
	          line,
	          len,
	          src,
	          get_line_len(src + len),
	          src + len);
	error_msg_v(fmt, args);
}

void parse_mark(Weft_ParseState *P)
{
	gc_mark(P->start);

	map_mark(P->map);
	for (size_t i = sizeof(Weft_Map *); i <= buf_at(P->map_stack);
	     i += sizeof(Weft_Map *)) {
		Weft_Map **map_p = buf_peek(P->map_stack, i);
		map_mark(*map_p);
	}

	for (size_t i = sizeof(Weft_Fn *); i <= buf_at(P->def_stack);
	     i += sizeof(Weft_Fn *)) {
		Weft_Fn **fn_p = buf_peek(P->def_stack, i);
		fn_mark(*fn_p);
	}

	list_mark(P->list);
	for (size_t i = sizeof(Weft_List *); i <= buf_at(P->list_stack);
	     i += sizeof(Weft_List *)) {
		Weft_List **list_p = buf_peek(P->list_stack, i);
		list_mark(*list_p);
	}

	for (size_t i = sizeof(char *); i <= buf_at(P->shuffle_in);
	     i += sizeof(char *)) {
		char **name_p = buf_peek(P->shuffle_in, i);
		gc_mark(*name_p);
	}
}

#define len_of(str) (sizeof(str) - 1)

static void output_data(Weft_ParseState *P, Weft_Data data)
{
	while (data_type(data) == WEFT_DATA_FN && !fn_is_builtin(data_ptr(data))
	       && fn_ptr(data_ptr(data)) && !list_cdr(fn_ptr(data_ptr(data)))) {
		data = list_car(fn_ptr(data_ptr(data)));
	}

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
	if (!P->word) {
		return;
	}

	Weft_Fn *fn = map_lookup(P->map, P->word, P->word_len);
	if (fn) {
		output_data(P, data_tag_ptr(WEFT_DATA_FN, fn));
	} else {
		parse_error(
			P, P->word, P->word_len, "%.*s is undefined", P->word_len, P->word);
	}

	P->word = NULL;
	P->word_len = 0;
}

static void
handle_shuffle_in(Weft_ParseState *P, const char *word, size_t word_len)
{
	if (buf_at(P->shuffle_in) / sizeof(char *) > UCHAR_MAX) {
		return parse_error(P,
		                   word,
		                   word_len,
		                   "Shuffle lambdas can contain a maximum arity of %u",
		                   UCHAR_MAX);
	}

	char *token = gc_alloc(word_len + 1);
	memcpy(token, word, word_len);
	token[word_len] = 0;

	P->shuffle_in = buf_push(P->shuffle_in, &token, sizeof(char *));
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
	char **name_p = buf_raw(P->shuffle_in);
	int len = buf_at(P->shuffle_in) / sizeof(char *);

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
	int index = get_shuffle_index(P, word, word_len);
	if (index < 0) {
		return parse_error(
			P, word, word_len, "%.*s not defined", word_len, word);
	} else {
		uint8_t byte = index;
		P->shuffle_out = buf_push(P->shuffle_out, &byte, sizeof(uint8_t));
	}
}

static void handle_word(Weft_ParseState *P, const char *word, size_t word_len)
{
	if (P->shuffle_mode == WEFT_PARSE_SHUFFLE_IN) {
		return handle_shuffle_in(P, word, word_len);
	} else if (P->shuffle_mode == WEFT_PARSE_SHUFFLE_OUT) {
		return handle_shuffle_out(P, word, word_len);
	}

	flush_word(P);
	P->word = word;
	P->word_len = word_len;
}

static void handle_def(Weft_ParseState *P, const char *at)
{
	if (P->shuffle_mode) {
		return parse_error(P, at, len_of(":"), "Expected shuffle lambda");
	} else if (!P->word) {
		return parse_error(P, at, len_of(":"), "Expected word before :");
	} else if (P->nest) {
		return parse_error(
			P, at, len_of(":"), "Cannot define terms inside of a quotation");
	}

	Weft_Fn *fn = new_fn_list(P->word, P->word_len, NULL);

	P->map = map_insert(P->map, fn);
	P->map_stack = buf_push(P->map_stack, &P->map, sizeof(Weft_Map *));

	P->word = NULL;
	P->word_len = 0;
	P->def_stack = buf_push(P->def_stack, &fn, sizeof(Weft_Fn *));

	P->list_stack = buf_push(P->list_stack, &P->list, sizeof(Weft_List *));
	P->list = NULL;
	P->node = NULL;

	P->indent_stack = buf_push(P->indent_stack, &P->indent, sizeof(size_t));
}

static void handle_dedent(Weft_ParseState *P)
{
	P->map_stack = buf_pop(P->map_stack, &P->map, sizeof(Weft_Map *));

	Weft_Fn *fn;
	P->def_stack = buf_pop(P->def_stack, &fn, sizeof(Weft_Fn *));
	fn_set_list(fn, P->list);

	P->list_stack = buf_pop(P->list_stack, &P->list, sizeof(Weft_List *));
	P->node = list_end(P->list);

	P->indent_stack = buf_drop(P->indent_stack, sizeof(size_t));
}

static bool is_dedent(const Weft_ParseState *P, size_t indent)
{
	if (!buf_at(P->indent_stack)) {
		return false;
	}

	size_t *prev_indent_p = buf_peek(P->indent_stack, sizeof(size_t));
	return *prev_indent_p >= indent;
}

static void handle_indent(Weft_ParseState *P, size_t indent)
{
	if (P->shuffle_mode) {
		return;
	}

	flush_word(P);
	while (is_dedent(P, indent)) {
		handle_dedent(P);
	}
	P->indent = indent;
}

static void handle_list_open(Weft_ParseState *P, const char *src)
{
	if (P->shuffle_mode) {
		return parse_error(P, src, len_of("["), "Expected shuffle lambda");
	}

	flush_word(P);
	P->list_stack = buf_push(P->list_stack, &P->list, sizeof(Weft_List *));
	P->list = NULL;
	P->node = NULL;
	P->nest += 1;
}

static void handle_list_close(Weft_ParseState *P, const char *at)
{
	if (P->shuffle_mode) {
		return parse_error(P, at, len_of("]"), "Expected shuffle lambda");
	}

	flush_word(P);
	if (!P->nest) {
		return parse_error(P, at, len_of("]"), "Expected [ before ]");
	}

	Weft_List *list = P->list;
	P->list_stack = buf_pop(P->list_stack, &P->list, sizeof(Weft_List *));
	P->node = list_end(P->list);
	
	output_data(P, data_tag_ptr(WEFT_DATA_LIST, list));
	P->nest -= 1;
}

static void handle_shuffle_open(Weft_ParseState *P, const char *at)
{
	if (P->shuffle_mode != WEFT_PARSE_SHUFFLE_OFF) {
		return parse_error(
			P, at, len_of("{"), "Expected word or -- in shuffle lambda");
	}

	flush_word(P);
	P->shuffle_mode = WEFT_PARSE_SHUFFLE_IN;
}

static void handle_shuffle_close(Weft_ParseState *P, const char *at)
{
	if (P->shuffle_mode == WEFT_PARSE_SHUFFLE_OFF) {
		return parse_error(P, at, len_of("}"), "Expected { before }");
	} else if (P->shuffle_mode == WEFT_PARSE_SHUFFLE_IN) {
		P->shuffle_mode = WEFT_PARSE_SHUFFLE_OFF;
		return parse_error(P, at, len_of("}"), "Expected -- before }");
	}
	P->shuffle_mode = WEFT_PARSE_SHUFFLE_OFF;

	Weft_Shuffle *shuffle =
		new_shuffle_empty(buf_at(P->shuffle_in) / sizeof(char *),
	                      buf_at(P->shuffle_out) / sizeof(uint8_t));
	for (unsigned i = 0; i < shuffle_out_count(shuffle); i++) {
		uint8_t *out = buf_raw(P->shuffle_out);
		shuffle_set_out(shuffle, i, out[i]);
	}

	buf_clear(P->shuffle_in);
	buf_clear(P->shuffle_out);

	output_data(P, data_tag_ptr(WEFT_DATA_SHUFFLE, shuffle));
}

static bool is_line_empty(const char *src)
{
	size_t i = 0;
	while (true) {
		if (!src[i] || src[i] == '\n' || src[i] == '#') {
			return true;
		} else if (!isspace(src[i])) {
			return false;
		}
		i++;
	}
}

static size_t parse_empty(const char *src)
{
	size_t len = 0;
	while (true) {
		if (src[len] == '\n') {
			if (!is_line_empty(src + len + 1)) {
				return len;
			}
			len++;
		} else if (src[len] == '#') {
			while (src[len] && src[len] != '\n') {
				len++;
			}
		} else if (isspace(src[len])) {
			len++;
		} else {
			return len;
		}
	}
}

static size_t parse_indent(const char *src)
{
	size_t len = 0;
	while (isspace(src[len])) {
		len++;
	}
	return len;
}

static bool is_delim(const char c)
{
	return !c || isspace(c) || strchr(delim_list, c);
}

static size_t parse_word(const char *src)
{
	size_t len = 0;
	while (!is_delim(src[len])) {
		len++;
	}
	return len;
}

static bool is_num(const char *src)
{
	if (*src == '-') {
		src++;
	}

	if (*src == '.') {
		src++;
	}
	return isdigit(*src);
}

static size_t handle_num(Weft_ParseState *P, const char *src)
{
	flush_word(P);

	size_t len = 0;
	bool negative = false;

	if (src[0] == '-') {
		len = 1;
		negative = true;
	}

	double left = 0.0;
	double right = 0.0;
	unsigned place = 0;
	bool dot = false;

	while (src[len] == '.' || isdigit(src[len])) {
		if (src[len] == '.') {
			if (dot) {
				parse_error(P, src, 1, "Expected digit");
				while (!is_delim(src[len])) {
					len++;
				}
				return len;
			}
			dot = true;
		} else if (dot) {
			right = (10.0 * right) + (double)(src[len] - '0');
			place++;
		} else {
			left = (10.0 * left) + (double)(src[len] - '0');
		}
		len++;
	}

	if (!is_delim(src[len])) {
		parse_error(P, src, 1, "Expected digit");
		while (!is_delim(src[len])) {
			len++;
		}
		return len;
	}

	while (place) {
		right /= 10.0;
		place--;
	}
	left += right;

	if (negative) {
		left = -left;
	}

	output_data(P, data_tag_num(left));
	return len;
}

Weft_List *parse(Weft_ParseState *P, char *src)
{
	P->start = src;

	size_t len;
	if (is_line_empty(src)) {
		src += parse_empty(src);
	} else {
		len = parse_indent(src);
		src += len;
		handle_indent(P, len);
	}

	while (*src) {
		switch (*src) {
		case '\n':
			len = parse_indent(src);
			src += len;
			handle_indent(P, len - len_of("\n"));
			break;
		case ':':
			handle_def(P, src);
			src += len_of(":");
			break;
		case '[':
			handle_list_open(P, src);
			src += len_of("[");
			break;
		case ']':
			handle_list_close(P, src);
			src += len_of("]");
			break;
		case '{':
			handle_shuffle_open(P, src);
			src += len_of("{");
			break;
		case '}':
			handle_shuffle_close(P, src);
			src += len_of("}");
			break;
		default:
			if (is_num(src)) {
				src += handle_num(P, src);
			} else if (P->shuffle_mode == WEFT_PARSE_SHUFFLE_IN && src[0] == '-'
			           && src[1] == '-') {
				src += len_of("--");
				P->shuffle_mode = WEFT_PARSE_SHUFFLE_OUT;
			} else {
				len = parse_word(src);
				handle_word(P, src, len);
				src += len;
			}
			break;
		}
		src += parse_empty(src);

		if (gc_is_ready()) {
			parse_mark(P);
			gc_collect();
		}
	}

	flush_word(P);
	error_flush();

	if (buf_at(P->list_stack)) {
		return NULL;
	}

	Weft_List *list = P->list;
	P->list = NULL;
	P->node = NULL;

	list_mark(list);

	return list;
}
