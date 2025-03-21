#ifndef WEFT_WORD_H
#define WEFT_WORD_H

#include <stddef.h>
#include <stdio.h>

// Forward Declarations

typedef struct weft_word Weft_Word;

// Data Types

struct weft_word {
	const char *src;
	size_t len;
};

// Functions

Weft_Word new_word(const char *src, size_t len);
void word_print(const Weft_Word word);
int word_compare(const Weft_Word left, const Weft_Word right);

#endif
