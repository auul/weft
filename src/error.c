#include "error.h"

#include <stdarg.h>
#include <stdio.h>

#define len_of(str) (sizeof(str) - 1)

int error_alloc(void)
{
	perror(WEFT_ERROR_ALLOC_MSG);
	return WEFT_ERROR_ALLOC_EXIT;
}

const char *get_src_line(size_t *line_no, const char *start, const char *src)
{
	const char *line = start;
	*line_no = 0;

	while (start < src) {
		if (*start == '\n') {
			line = start + len_of("\n");
			*line_no += 1;
		}
		start++;
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

void error_parse(const char *path,
                 const char *start,
                 const char *src,
                 size_t len,
                 const char *fmt,
                 ...)
{
	size_t line_no;
	const char *line = get_src_line(&line_no, start, src);

	fprintf(
		stderr, "%s:%zu:%zu: \e[91merror:\e[0m ", path, line_no, src - line);

	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);

	fprintf(stderr, "\n%5zu | %.*s\e[91m", line_no, src - line, line);
	while (len) {
		if (*src == '\n') {
			line_no++;
			fprintf(stderr, "\e[0m\n%5zu | \e[91m", line_no);
		} else {
			fprintf(stderr, "%c", *src);
		}
		src++;
		len--;
	}

	fprintf(stderr, "\e[0m");
	while (*src && *src != '\n') {
		fprintf(stderr, "%c", *src);
		src++;
	}
	fprintf(stderr, "\n");
}
