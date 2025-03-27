#include "debug.h"

#ifndef WEFT_FILE_H
#define WEFT_FILE_H

#include <stdio.h>

#ifdef WEFT_DEBUG_ALL
#define WEFT_DEBUG_FILE
#endif

// Functions

char *file_load_text(const char *path);
char *file_load_text_f(FILE *f);

#endif
