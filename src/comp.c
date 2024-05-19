#include "comp.h"

float comp_abs_no_sqrt(vec2 z)
{
	return z.x * z.x + z.y * z.y;
}

vec2 comp_square(vec2 z)
{
	return (vec2) { z.x * z.x - z.y * z.y, 2 * z.x * z.y };
}

vec2 comp_add(vec2 z1, vec2 z2)
{
	return (vec2) { z1.x + z2.x, z1.y + z2.y };
}
