#include "shuffle.h"
#include "gc.h"

#include <stdio.h>

Weft_Shuffle *new_shuffle(uint8_t in_count, uint8_t out_count, ...)
{
	va_list args;
	va_start(args, out_count);
	Weft_Shuffle *shuffle = new_shuffle_v(in_count, out_count, args);
	va_end(args);

	return shuffle;
}

Weft_Shuffle *new_shuffle_v(uint8_t in_count, uint8_t out_count, va_list args)
{
	Weft_Shuffle *shuffle = new_shuffle_empty(in_count, out_count);
	for (unsigned i = 0; i < out_count; i++) {
		shuffle_set_out(shuffle, i, va_arg(args, int));
	}
	return shuffle;
}

Weft_Shuffle *new_shuffle_empty(uint8_t in_count, uint8_t out_count)
{
	Weft_Shuffle *shuffle =
		gc_alloc(sizeof(Weft_Shuffle) + out_count * sizeof(uint8_t));
	shuffle->in_count = in_count;
	shuffle->out_count = out_count;

	return shuffle;
}

unsigned shuffle_in_count(const Weft_Shuffle *shuffle)
{
	return shuffle->in_count;
}

unsigned shuffle_out_count(const Weft_Shuffle *shuffle)
{
	return shuffle->out_count;
}

unsigned shuffle_out_value(const Weft_Shuffle *shuffle, uint8_t index)
{
	return shuffle->out[index];
}

void shuffle_set_out(Weft_Shuffle *shuffle, uint8_t index, uint8_t value)
{
	shuffle->out[index] = value;
}

void shuffle_print(const Weft_Shuffle *shuffle)
{
	printf("{");
	for (unsigned i = 0; i < shuffle->in_count; i++) {
		printf("%u ", i);
	}
	printf("--");
	for (unsigned i = 0; i < shuffle->out_count; i++) {
		printf(" %u", shuffle->out[i]);
	}
	printf("}");
}

void shuffle_mark(Weft_Shuffle *shuffle)
{
	gc_mark(shuffle);
}

bool shuffle_equal(const Weft_Shuffle *left, const Weft_Shuffle *right)
{
	if (left == right) {
		return true;
	} else if (left->in_count != right->in_count
	           || left->out_count != right->out_count) {
		return false;
	}

	for (unsigned i = 0; i < left->out_count; i++) {
		if (left->out[i] != right->out[i]) {
			return false;
		}
	}
	return true;
}

