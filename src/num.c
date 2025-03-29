#include "num.h"
#include "str.h"

#include <math.h>

Weft_Str *num_stringify(double fnum)
{
	return str_printf("%g", fnum);
}

bool num_is_equal(double left, double right)
{
	return fabs(left - right) < WEFT_NUM_EPSILON;
}
