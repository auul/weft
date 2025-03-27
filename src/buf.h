#ifndef WEFT_BUF_H
#define WEFT_BUF_H

#include <stddef.h>
#include <stdint.h>

#ifdef WEFT_DEBUG_ALL
#define WEFT_DEBUG_BUF
#endif

// Forward Declarations

typedef struct weft_buf Weft_Buf;

// Data Types

struct weft_buf {
	size_t cap;
	size_t at;
	char raw[];
};

// Functions

Weft_Buf *new_buf(size_t cap);
size_t buf_cap(const Weft_Buf *buf);
size_t buf_at(const Weft_Buf *buf);
void *buf_raw(Weft_Buf *buf);
Weft_Buf *buf_clear(Weft_Buf *buf);
Weft_Buf *buf_push(Weft_Buf *buf, void *src, size_t size);
Weft_Buf *buf_push_byte(Weft_Buf *buf, uint8_t byte);
Weft_Buf *buf_pop(Weft_Buf *buf, void *dest, size_t size);
Weft_Buf *buf_drop(Weft_Buf *buf, size_t size);
void *buf_peek(Weft_Buf *buf, size_t size);

#endif
