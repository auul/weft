#include "debug.h"

#ifndef WEFT_NUM_H
#define WEFT_NUM_H

#include <stdbool.h>

#ifdef WEFT_DEBUG_ALL
#define WEFT_DEBUG_NUM
#endif

// Constants

static const double WEFT_NUM_EPSILON = 0.000000000000001;

// Functions

void num_print(const double num);
bool num_equal(const double left, const double right);

#endif
