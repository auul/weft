#include "debug.h"

#ifndef WEFT_BOOL_H
#define WEFT_BOOL_H

#include <stdbool.h>

#ifdef WEFT_DEBUG_ALL
#define WEFT_DEBUG_BOOL
#endif

// Functions

void bool_print(const bool bnum);
bool bool_equal(const bool left, const bool right);

#endif
