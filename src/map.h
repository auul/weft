#ifndef WEFT_MAP_H
#define WEFT_MAP_H

// Forward Declarations

typedef struct weft_map Weft_Map;
typedef struct weft_fn Weft_Fn;

// Local Includes

#include "word.h"

// Data Types

struct weft_map {
	Weft_Fn *key;
	Weft_Map *left;
	Weft_Map *right;
};

// Functions

void map_print(const Weft_Map *map);
void map_print_bare(const Weft_Map *map);
void map_mark(const Weft_Map *map);
Weft_Fn *map_lookup(Weft_Map *map, const Weft_Word word);
Weft_Map *map_insert(Weft_Map *map, Weft_Fn *key);
Weft_Fn *map_touch(Weft_Map **map_p, const Weft_Word word);

#endif
