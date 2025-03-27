#include "error.h"
#include "buf.h"

#include <stdio.h>
#include <stdlib.h>

// Globals

Weft_Buf *error_list;

// Functions

void error_init(void)
{
	error_list = new_buf(sizeof(char *));
}

void error_exit(void)
{
	if (error_list) {
		free(error_list);
		error_list = NULL;
	}
}

int error_alloc(void)
{
	error_flush();
	perror(ERROR_ALLOC_MSG);
	return ERROR_ALLOC_EXIT;
}

static void print_msg(const char *msg)
{
	printf("%s\n", msg);
}

void error_flush(void)
{
	if (!error_list) {
		return;
	}

	char **msg = buf_raw(error_list);
	size_t len = buf_at(error_list) / sizeof(char *);

	for (size_t i = 0; i < len; i++) {
		print_msg(msg[i]);
		free(msg[i]);
	}
	error_list = buf_clear(error_list);
}

bool error_msg(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	error_msg_v(fmt, args);
	va_end(args);

	return false;
}

static size_t get_msg_len(const char *fmt, va_list args_src)
{
	va_list args;
	va_copy(args, args_src);
	size_t len = vsnprintf(NULL, 0, fmt, args) + 1;
	va_end(args);

	return len;
}

static void push_msg(char *msg)
{
	if (!error_list) {
		error_list = new_buf(sizeof(char *));
	}
	error_list = buf_push(error_list, &msg, sizeof(char *));
}

bool error_msg_v(const char *fmt, va_list args)
{
	size_t len = get_msg_len(fmt, args);
	char *msg = malloc(len + 1);
	if (!msg) {
		exit(error_alloc());
	}

	vsnprintf(msg, len, fmt, args);
	push_msg(msg);

	return false;
}
