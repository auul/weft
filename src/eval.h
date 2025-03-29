#ifndef WEFT_EVAL_H
#define WEFT_EVAL_H

#include <stddef.h>

// Forward Declarations

typedef struct weft_buf Weft_Buf;
typedef struct weft_list Weft_List;
typedef struct weft_eval_state Weft_EvalState;

// Local Includes

#include "data.h"

// Data Types

struct weft_eval_state {
	Weft_List *ctrl;
	Weft_Buf *stack;
	Weft_Buf *nest;
};

// Functions

void eval_init(Weft_EvalState *W);
void eval_exit(Weft_EvalState *W);
void eval_mark(Weft_EvalState *W);
size_t eval_get_stack_size(const Weft_EvalState *W);
bool eval_type_mismatch(Weft_EvalState *W, size_t index, Weft_DataType type);
void eval_push_stack(Weft_EvalState *W, Weft_Data data);
Weft_Data eval_pop_stack(Weft_EvalState *W);
Weft_Data eval_peek_stack(Weft_EvalState *W, size_t index);
void eval_push_ctrl(Weft_EvalState *W, Weft_Data data);
bool eval_list(Weft_EvalState *W, Weft_List *list);
static bool eval_fn(Weft_EvalState *W, Weft_Fn *fn);
static bool eval_shuffle(Weft_EvalState *W, Weft_Shuffle *shuffle);
bool eval(Weft_EvalState *W, Weft_List *ctrl);

#endif
