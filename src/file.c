#include "file.h"
#include "buf.h"
#include "error.h"
#include "gc.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

char *file_load_text(const char *path)
{
	FILE *f = fopen(path, "r");
	if (!f) {
		error_msg("Unable to open file '%s': %s", path, strerror(errno));
		return NULL;
	}

	char *text = file_load_text_f(f);
	fclose(f);

	return text;
}

static void push_byte(size_t *cap_p, size_t *at_p, char **buf_p, char c)
{
	size_t cap = *cap_p;
	size_t at = *at_p;
	char *buf = *buf_p;

	if (at == cap) {
		char *re_buf = gc_alloc(2 * cap);
		memcpy(re_buf, buf, at);
		buf = re_buf;
		*buf_p = buf;
		*cap_p = 2 * cap;
	}

	buf[at] = c;
	*at_p = at + 1;
}

char *file_load_text_f(FILE *f)
{
	size_t cap = 1;
	size_t at = 0;
	char *buf = gc_alloc(cap);

	int c = fgetc(f);
	while (c != -1) {
		push_byte(&cap, &at, &buf, c);
		c = fgetc(f);
	}
	push_byte(&cap, &at, &buf, 0);

	return buf;
}
