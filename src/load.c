#include "load.h"
#include "error.h"

#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

char *load_text(const char *path)
{
	FILE *f = fopen(path, "r");
	if (!f) {
		error_msg("%s", strerror(errno));
		return NULL;
	}
	char *text = load_text_f(f);
	fclose(f);

	return text;
}

static bool expand_text(size_t *cap_p, char **text_p)
{
	char *text = *text_p;
	size_t cap = *cap_p;

	cap *= 2;
	text = realloc(text, cap);
	if (!text) {
		exit(error_alloc());
	}

	*cap_p = cap;
	*text_p = text;

	return true;
}

static bool push_char(size_t *cap_p, size_t *at_p, char **text_p, const char c)
{
	if (*at_p == *cap_p) {
		expand_text(cap_p, text_p);
	}

	char *text = *text_p;
	size_t at = *at_p;

	text[at] = c;
	*at_p = at + 1;

	return true;
}

char *load_text_f(FILE *f)
{
	size_t cap = LOAD_INIT_TEXT_CAP;
	size_t at = 0;
	char *text = malloc(cap);
	if (!text) {
		exit(error_alloc());
	}

	while (!feof(f)) {
		push_char(&cap, &at, &text, fgetc(f));
	}
	push_char(&cap, &at, &text, 0);

	return text;
}
