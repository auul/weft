#include "gc.h"
#include "error.h"

#include <stdlib.h>

#ifdef WEFT_DEBUG_GC
#include <stdio.h>
#endif

// Globals

static Weft_GC *gc_heap;
size_t gc_base = WEFT_GC_INIT_BASE;
size_t gc_count = 0;

// Functions

static Weft_GC *get_tag(const void *ptr)
{
	return (Weft_GC *)ptr - 1;
}

static Weft_GC *get_tag_prev(const Weft_GC *tag)
{
	return (Weft_GC *)(tag->prev >> 1);
}

static void set_tag_prev(Weft_GC *tag, Weft_GC *prev)
{
	tag->prev = (uintptr_t)prev << 1;
}

static bool is_tag_marked(const Weft_GC *tag)
{
	return tag->prev & 1;
}

static void mark_tag(Weft_GC *tag)
{
	tag->prev |= 1;
}

static void unmark_tag(Weft_GC *tag)
{
	tag->prev &= ~((uintptr_t)1);
}

void *gc_alloc(size_t size)
{
	Weft_GC *tag = malloc(sizeof(Weft_GC) + size);
	if (!tag) {
		exit(error_alloc());
	}

	unmark_tag(tag);
	set_tag_prev(tag, gc_heap);
	gc_heap = tag;
	gc_count++;

	return tag->ptr;
}

bool gc_mark(const void *ptr)
{
	if (!ptr) {
		return true;
	}

	Weft_GC *tag = get_tag(ptr);
	if (is_tag_marked(tag)) {
		return true;
	}
	mark_tag(tag);

	return false;
}

static Weft_GC *pop_tag(Weft_GC *tag)
{
	Weft_GC *prev = get_tag_prev(tag);
	free(tag);
	gc_count--;
	return prev;
}

static void pop_heap(void)
{
	gc_heap = pop_tag(gc_heap);
}

void gc_collect(void)
{
#ifdef WEFT_DEBUG_GC
	printf("Collecting Garbage... ");
	size_t start = gc_count;
#endif

	while (gc_heap && !is_tag_marked(gc_heap)) {
		pop_heap();
	}

	for (Weft_GC *tag = gc_heap; tag; tag = get_tag_prev(tag)) {
		while (get_tag_prev(tag) && !is_tag_marked(get_tag_prev(tag))) {
			set_tag_prev(tag, pop_tag(get_tag_prev(tag)));
		}
		unmark_tag(tag);
	}

#ifdef WEFT_DEBUG_GC
	printf("%zu nodes collected\n", start - gc_count);
#endif
	gc_base = gc_count;
}

bool gc_is_ready(void)
{
	return gc_count >= 2 * gc_base;
}

#ifdef WEFT_DEBUG_GC
size_t gc_get_count(void)
{
	return gc_count;
}
#endif
