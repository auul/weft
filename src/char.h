#ifndef WEFT_CHAR_H
#define WEFT_CHAR_H

#include <stdbool.h>

// Forward Declarations

typedef struct weft_str Weft_Str;

// Functions

bool char_is_printable(unsigned char c);
Weft_Str *char_stringify(unsigned char c);
Weft_Str *char_stringify_esc(unsigned char c);
unsigned char char_get_esc(unsigned char c);
bool char_is_equal(unsigned char left, unsigned char right);

#endif
