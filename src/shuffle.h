#ifndef WEFT_SHUFFLE_H
#define WEFT_SHUFFLE_H

#include <stdbool.h>
#include <stdint.h>

// Forward Declarations

typedef struct weft_str Weft_Str;
typedef struct weft_shuffle Weft_Shuffle;

// Data Types

struct weft_shuffle {
	uint8_t in_count;
	uint8_t out_count;
	uint8_t out[];
};

// Functions

Weft_Shuffle *new_shuffle(uint8_t in_count, uint8_t out_count, ...);
Weft_Shuffle *new_shuffle_empty(uint8_t in_count, uint8_t out_count);
unsigned shuffle_get_in_count(const Weft_Shuffle *shuffle);
unsigned shuffle_get_out_count(const Weft_Shuffle *shuffle);
unsigned shuffle_get_out_at(const Weft_Shuffle *shuffle, uint8_t index);
void shuffle_set_out_at(Weft_Shuffle *shuffle, uint8_t index, uint8_t value);
void shuffle_mark(Weft_Shuffle *shuffle);
Weft_Str *shuffle_stringify(const Weft_Shuffle *shuffle);
bool shuffle_is_equal(const Weft_Shuffle *left, const Weft_Shuffle *right);

#endif
