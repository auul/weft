#include "debug.h"

#ifndef WEFT_BI_H
#define WEFT_BI_H

#ifdef WEFT_DEBUG_ALL
#define WEFT_DEBUG_BI
#endif

// Forward Declarations

typedef struct weft_eval_state Weft_EvalState;
typedef struct weft_map Weft_Map;

// Functions

Weft_Map *bi_init(void);

#endif
