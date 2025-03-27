#include "error.h"
#include "eval.h"
#include "file.h"
#include "gc.h"
#include "list.h"
#include "parse.h"

#include <stdlib.h>

int main(int argc, char **args)
{
	if (argc < 2) {
		return 0;
	}

	error_init();

	char *text = file_load_text(args[1]);
	if (!text) {
		error_flush();
		error_exit();
		return 0;
	}

	Weft_ParseState P;
	parse_init(&P);
	Weft_List *ctrl = parse(&P, text);
	parse_exit(&P);

	list_mark(ctrl);
	gc_collect();

	Weft_EvalState W;
	eval_init(&W);
	eval(&W, ctrl);
	eval_exit(&W);

	error_exit();

	return 0;
}
