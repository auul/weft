#include "eval.h"
#include "file.h"
#include "list.h"
#include "parse.h"
#include "str.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **args)
{
	if (argc < 2) {
		return 0;
	}

	char *text = file_load_text(args[1]);
	if (!text) {
		return 0;
	}

	Weft_ParseState P;
	parse_init(&P);
	Weft_List *ctrl = parse(&P, args[1], text);
	parse_exit(&P);
	free(text);

	Weft_EvalState W;
	eval_init(&W);
	eval(&W, ctrl);
	eval_exit(&W);

	return 0;
}
