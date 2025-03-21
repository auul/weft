#include "word.h"

Weft_Word new_word(const char *src, size_t len)
{
	Weft_Word word = {
		.src = src,
		.len = len,
	};
	return word;
}

void word_print(const Weft_Word word)
{
	printf("%.*s", word.len, word.src);
}

int word_compare(const Weft_Word left, const Weft_Word right)
{
	for (size_t i = 0; i < left.len; i++) {
		if (i > right.len || left.src[i] > right.src[i]) {
			return -1;
		} else if (left.src[i] < right.src[i]) {
			return 1;
		}
	}

	if (right.len > left.len) {
		return 1;
	}
	return 0;
}
