#ifndef WEFT_ERROR_H
#define WEFT_ERROR_H

#include <stdbool.h>
#include <stddef.h>

// Constants

static const char WEFT_ERROR_ALLOC_MSG[] = "Could not allocate memory";
static const int WEFT_ERROR_ALLOC_EXIT = 1;

// Functions

int error_alloc(void);
void error_parse(const char *path,
                 const char *start,
                 const char *src,
                 size_t len,
                 const char *fmt,
                 ...);

#endif
