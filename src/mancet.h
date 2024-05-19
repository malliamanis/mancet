#ifndef MANCET_H
#define MANCET_H

#include <stdint.h>

typedef struct {
	float x, y;
} vec2;

void mancet_run(uint32_t window_width, uint32_t window_height, uint32_t pixel_width);

#endif
