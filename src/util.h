#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>

typedef struct {
	double x, y;
} vec2;

uint32_t util_hsv_to_rgb(float hue, float saturation, float value);

#endif
