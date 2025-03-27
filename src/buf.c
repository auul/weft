#include "buf.h"
#include "error.h"

#include <stdlib.h>
#include <string.h>

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

size_t buf_cap(const Weft_Buf *buf)
{
	return buf->cap;
}

size_t buf_at(const Weft_Buf *buf)
{
	return buf->at;
}

void *buf_raw(Weft_Buf *buf)
{
	return buf->raw;
}

static Weft_Buf *realloc_buf(Weft_Buf *buf, size_t cap)
{
	buf = realloc(buf, sizeof(Weft_Buf) + cap);
	if (!buf) {
		exit(error_alloc());
	}
	buf->cap = cap;

	return buf;
}

Weft_Buf *buf_clear(Weft_Buf *buf)
{
	buf->at = 0;
	return buf;
}

Weft_Buf *buf_push(Weft_Buf *buf, void *src, size_t size)
{
	if (buf->at + size > buf->cap) {
		buf = realloc_buf(buf, 2 * (buf->at + size));
	}

	memcpy(buf->raw + buf->at, src, size);
	buf->at += size;

	return buf;
}

Weft_Buf *buf_push_byte(Weft_Buf *buf, uint8_t byte)
{
	if (buf->at == buf->cap) {
		buf = realloc_buf(buf, 2 * buf->cap);
	}

	buf->raw[buf->at] = byte;
	buf->at += 1;

	return buf;
}

Weft_Buf *buf_pop(Weft_Buf *buf, void *dest, size_t size)
{
	buf->at -= size;
	memcpy(dest, buf->raw + buf->at, size);

	return buf;
}

Weft_Buf *buf_drop(Weft_Buf *buf, size_t size)
{
	buf->at -= size;
	return buf;
}

void *buf_peek(Weft_Buf *buf, size_t size)
{
	return buf->raw + buf->at - size;
}
