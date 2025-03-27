#include "debug.h"

#ifndef WEFT_GC_H
#define WEFT_GC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef WEFT_DEBUG_ALL
#define WEFT_DEBUG_GC
#endif

// Forward Declarations

typedef struct weft_gc Weft_GC;

// Data Structures

struct weft_gc {
	uintptr_t prev;
	char ptr[];
};

// Constants

static const size_t WEFT_GC_INIT_BASE = 8;

// Functions

void *gc_alloc(size_t size);
bool gc_mark(const void *ptr);
void gc_collect(void);
bool gc_is_ready(void);
#ifdef WEFT_DEBUG_GC
size_t gc_get_count(void);
#endif

#endif
