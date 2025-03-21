#include "error.h"
#include "eval.h"
#include "gc.h"
#include "list.h"
#include "load.h"
#include "parse.h"

#include <stdlib.h>

int main(int argc, char **args)
{
	if (argc < 2) {
		return 0;
	}

	error_init();

	char *text = load_text(args[1]);
	if (!text) {
		error_flush();
		return 0;
	}

	Weft_ParseState P;
	parse_init(&P);
	Weft_List *ctrl = parse(&P, text);
	parse_exit(&P);

	list_mark(ctrl);
	printf("%zu\n", gc_count());
	gc_collect();
	printf("%zu\n", gc_count());

	Weft_EvalState W;
	eval_init(&W);
	if (!eval(&W, ctrl)) {
		error_flush();
	}
	eval_exit(&W);

	gc_collect();

	return 0;
}
