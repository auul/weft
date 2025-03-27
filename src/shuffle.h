#include "debug.h"

#ifndef WEFT_SHUFFLE_H
#define WEFT_SHUFFLE_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef WEFT_DEBUG_ALL
#define WEFT_DEBUG_SHUFFLE
#endif

// Forward Declarations

typedef struct weft_shuffle Weft_Shuffle;

// Data Types

struct weft_shuffle {
	uint8_t in_count;
	uint8_t out_count;
	uint8_t out[];
};

// Functions

Weft_Shuffle *new_shuffle(uint8_t in_count, uint8_t out_count, ...);
Weft_Shuffle *new_shuffle_v(uint8_t in_count, uint8_t out_count, va_list args);
Weft_Shuffle *new_shuffle_empty(uint8_t in_count, uint8_t out_count);
unsigned shuffle_in_count(const Weft_Shuffle *shuffle);
unsigned shuffle_out_count(const Weft_Shuffle *shuffle);
unsigned shuffle_out_value(const Weft_Shuffle *shuffle, uint8_t index);
void shuffle_set_out(Weft_Shuffle *shuffle, uint8_t index, uint8_t value);
void shuffle_print(const Weft_Shuffle *shuffle);
void shuffle_mark(Weft_Shuffle *shuffle);
bool shuffle_equal(const Weft_Shuffle *left, const Weft_Shuffle *right);

#endif
