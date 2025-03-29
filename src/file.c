#include "file.h"
#include "error.h"

#include <stdio.h>
#include <stdlib.h>

char *file_load_text(const char *path)
{
	FILE *f = fopen(path, "r");
	if (!f) {
		fprintf(
			stderr, "\e[91merror:\e[0m File '%s' could not be opened\n", path);
		return NULL;
	}

	fseek(f, 0, SEEK_END);
	size_t len = ftell(f);
	fseek(f, 0, SEEK_SET);

	char *text = malloc(len + 1);
	if (!text) {
		perror(WEFT_ERROR_ALLOC_MSG);
		fclose(f);
		exit(WEFT_ERROR_ALLOC_EXIT);
	}

	fread(text, 1, len, f);
	text[len] = 0;
	fclose(f);

	return text;
}
