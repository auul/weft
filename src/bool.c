#include "bool.h"

#include <stdio.h>

void bool_print(const bool bnum)
{
	if (bnum) {
		printf("true");
	} else {
		printf("false");
	}
}

bool bool_equal(const bool left, const bool right)
{
	return left == right;
}
