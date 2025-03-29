#ifndef WEFT_NUM_H
#define WEFT_NUM_H

#include <stdbool.h>

// Forward Declarations

typedef struct weft_str Weft_Str;

// Constants

static const double WEFT_NUM_EPSILON = 0.0000000000001;

// Functions

Weft_Str *num_stringify(double fnum);
bool num_is_equal(double left, double right);

#endif
