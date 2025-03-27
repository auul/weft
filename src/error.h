#include "debug.h"

#ifndef WEFT_ERROR_H
#define WEFT_ERROR_H

#include <stdarg.h>
#include <stdbool.h>

#ifdef WEFT_DEBUG_ALL
#define WEFT_DEBUG_ERROR
#endif

// Constants

static const char ERROR_ALLOC_MSG[] = "Couldn't allocate memory";
static const int ERROR_ALLOC_EXIT = 1;

// Functions

void error_init(void);
void error_exit(void);
int error_alloc(void);
void error_flush(void);
bool error_msg(const char *fmt, ...);
bool error_msg_v(const char *fmt, va_list args);

#endif
