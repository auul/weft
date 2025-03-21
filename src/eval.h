#ifndef WEFT_EVAL_H
#define WEFT_EVAL_H

#include <stdbool.h>
#include <stddef.h>

// Forward Declarations

typedef struct weft_eval_state Weft_EvalState;
typedef struct weft_list Weft_List;
typedef struct weft_buf Weft_Buf;
typedef struct weft_fn Weft_Fn;

// Data Types

struct weft_eval_state {
	Weft_List *stack;
	Weft_List *ctrl;
	Weft_Buf *nest;
};

// Constants

static const size_t WEFT_EVAL_INIT_NEST_CAP = 1;

// Functions

void eval_init(Weft_EvalState *W);
void eval_exit(Weft_EvalState *W);
bool eval(Weft_EvalState *W, Weft_List *ctrl);
bool eval_fn(Weft_EvalState *W, Weft_Fn *fn);
bool eval_list(Weft_EvalState *W, Weft_List *list);

#endif
