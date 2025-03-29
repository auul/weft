#ifndef WEFT_MAP_H
#define WEFT_MAP_H

#include <stddef.h>

// Forward Declarations

typedef struct weft_map Weft_Map;
typedef struct weft_fn Weft_Fn;

// Data Types

struct weft_map {
	Weft_Fn *key;
	Weft_Map *left;
	Weft_Map *right;
};

// Functions

void map_print(const Weft_Map *map);
void map_mark(Weft_Map *map);
Weft_Fn *map_lookup(Weft_Map *map, const char *word, size_t word_len);
Weft_Map *map_insert(Weft_Map *map, Weft_Fn *key);

#endif
