#include <stdlib.h>
#include <stdint.h>

#include "mancet.h"

#define PIXEL_WIDTH  1
#define WIDTH        1280
#define HEIGHT      (WIDTH * 9 / 16)

int main(int argc, char **argv)
{
	uint32_t width, height, pixel_width;

	if (argc < 4) {
		width       = WIDTH;
		height      = HEIGHT;
		pixel_width = PIXEL_WIDTH;
	}
	else {
		width       = atoi(argv[1]);
		height      = atoi(argv[2]);
		pixel_width = atoi(argv[3]);
	}

	mancet_run(width, height, pixel_width);

	return 0;
}
