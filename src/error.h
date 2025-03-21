#ifndef WEFT_ERROR_H
#define WEFT_ERROR_H

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

// Constants

static const size_t ERROR_INIT_BUF_CAP = 1;

static const char ERROR_ALLOC_MSG[] = "Fatal Error: Could not allocate memory.";
static const int ERROR_ALLOC_EXIT_CODE = 1;

// Functions

void error_init(void);
void error_exit(void);
int error_alloc(void);
void error_flush(void);
bool error_msg(const char *fmt, ...);
bool error_msg_v(const char *fmt, va_list args);

#endif
