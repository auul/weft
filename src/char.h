#ifndef WEFT_CHAR_H
#define WEFT_CHAR_H

#include <stdbool.h>

// Forward Declarations

typedef struct weft_str Weft_Str;

// Functions

bool char_is_printable(char c);
Weft_Str *char_stringify(char c);
Weft_Str *char_stringify_esc(char c);
bool char_is_equal(char left, char right);

#endif
