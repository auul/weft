#include "error.h"
#include "buf.h"

#include <stdio.h>
#include <stdlib.h>

// Globals

Weft_Buf *error_buf;

// Functions

void error_init(void)
{
	error_buf = new_buf(ERROR_INIT_BUF_CAP * sizeof(char *));
}

void error_exit(void)
{
	if (!error_buf) {
		return;
	}

	error_flush();
	free(error_buf);
}

int error_alloc(void)
{
	perror(ERROR_ALLOC_MSG);
	return ERROR_ALLOC_EXIT_CODE;
}

void error_flush(void)
{
	for (size_t i = 0; i < buf_get_at(error_buf); i += sizeof(char *)) {
		char *msg = *((char **)(buf_get_raw(error_buf) + i));
		fprintf(stderr, "%s", msg);
		free(msg);
	}
	buf_clear(error_buf);
}

bool error_msg(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	error_msg_v(fmt, args);
	va_end(args);
}

static size_t get_msg_len(const char *fmt, va_list args_src)
{
	va_list args;
	va_copy(args, args_src);
	size_t len = vsnprintf(NULL, 0, fmt, args) + 1;
	va_end(args);

	return len;
}

bool error_msg_v(const char *fmt, va_list args)
{
	size_t len = get_msg_len(fmt, args);
	char *msg = malloc(len + 1);
	if (!msg) {
		exit(error_alloc());
	}
	vsnprintf(msg, len, fmt, args);
	error_buf = buf_push(error_buf, &msg, sizeof(char *));
}
