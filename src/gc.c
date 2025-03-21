#include "gc.h"
#include "error.h"

#include <stdlib.h>

// Globals

static Weft_GC *gc_heap;

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

void *gc_alloc(size_t size)
{
	Weft_GC *tag = malloc(sizeof(Weft_GC) + size);
	if (!tag) {
		exit(error_alloc());
	}

	set_tag_prev(tag, gc_heap);
	gc_heap = tag;

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

void gc_collect(void)
{
	while (gc_heap && !is_tag_marked(gc_heap)) {
		Weft_GC *re_heap = get_tag_prev(gc_heap);
		free(gc_heap);
		gc_heap = re_heap;
	}

	for (Weft_GC *tag = gc_heap; tag; tag = get_tag_prev(tag)) {
		while (get_tag_prev(tag) && !is_tag_marked(get_tag_prev(tag))) {
			Weft_GC *prev = get_tag_prev(tag);
			set_tag_prev(tag, get_tag_prev(prev));
			free(prev);
		}
	}
}

size_t gc_count(void)
{
	size_t count = 0;
	for (const Weft_GC *tag = gc_heap; tag; tag = get_tag_prev(tag)) {
		count++;
	}
	return count;
}
