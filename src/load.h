#ifndef WEFT_LOAD_H
#define WEFT_LOAD_H

#include <stdio.h>

// Constants

static const size_t LOAD_INIT_TEXT_CAP = 1;

// Functions

char *load_text(const char *path);
char *load_text_f(FILE *f);

#endif
