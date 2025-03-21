#include "buf.h"
#include "error.h"

#include <stdlib.h>
#include <string.h>

size_t buf_get_cap(const Weft_Buf *buf)
{
	return buf->cap;
}

size_t buf_get_at(const Weft_Buf *buf)
{
	return buf->at;
}

const char *buf_get_raw(const Weft_Buf *buf)
{
	return buf->raw;
}

Weft_Buf *new_buf(size_t cap)
{
	Weft_Buf *buf = malloc(sizeof(Weft_Buf) + cap);
	if (!buf) {
		exit(error_alloc());
	}

	buf->cap = cap;
	buf->at = 0;

	return buf;
}

void buf_clear(Weft_Buf *buf)
{
	buf->at = 0;
}

static Weft_Buf *expand_buf(Weft_Buf *buf, size_t size)
{
	buf->cap = 2 * (buf->at + size);
	buf = realloc(buf, sizeof(Weft_Buf) + buf->cap);
	if (!buf) {
		exit(error_alloc());
	}
	return buf;
}

Weft_Buf *buf_push(Weft_Buf *buf, const void *src, size_t size)
{
	if (buf->at + size > buf->cap) {
		buf = expand_buf(buf, size);
	}

	memcpy(buf->raw + buf->at, src, size);
	buf->at += size;

	return buf;
}

void *buf_pop(void *dest, Weft_Buf *buf, size_t size)
{
	buf->at -= size;
	memcpy(dest, buf->raw + buf->at, size);

	return dest;
}

const void *buf_peek(const Weft_Buf *buf, size_t size)
{
	return (const void *)(buf->raw + buf->at - size);
}
