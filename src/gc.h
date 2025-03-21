#ifndef WEFT_GC_H
#define WEFT_GC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Forward Declarations

typedef struct weft_gc Weft_GC;

// Data Structures

struct weft_gc {
	uintptr_t prev;
	char ptr[];
};

// Functions

void *gc_alloc(size_t size);
bool gc_mark(const void *ptr);
void gc_collect(void);
size_t gc_count(void);

#endif
