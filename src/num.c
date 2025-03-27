#include "num.h"

#include <math.h>
#include <stdio.h>

void num_print(const double num)
{
	printf("%g", num);
}

bool num_equal(const double left, const double right)
{
	return fabs(left - right) < WEFT_NUM_EPSILON;
}
