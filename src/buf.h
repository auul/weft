#ifndef WEFT_BUF_H
#define WEFT_BUF_H

#include <stddef.h>

// Forward Declarations

typedef struct weft_buf Weft_Buf;

// Data Types

struct weft_buf {
	size_t cap;
	size_t at;
	char raw[];
};

// Functions

size_t buf_get_cap(const Weft_Buf *buf);
size_t buf_get_at(const Weft_Buf *buf);
const char *buf_get_raw(const Weft_Buf *buf);
Weft_Buf *new_buf(size_t cap);
void buf_clear(Weft_Buf *buf);
Weft_Buf *buf_push(Weft_Buf *buf, const void *src, size_t size);
void *buf_pop(void *dest, Weft_Buf *buf, size_t size);
const void *buf_peek(const Weft_Buf *buf, size_t size);

#endif
