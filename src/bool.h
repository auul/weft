#ifndef WEFT_BOOL_H
#define WEFT_BOOL_H

#include <stdbool.h>

// Forward Declarations

typedef struct weft_str Weft_Str;

// Functions

Weft_Str *bool_stringify(bool b);
bool bool_is_equal(bool left, bool right);

#endif
