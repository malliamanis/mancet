#include <SDL2/SDL.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "comp.h"
#include "mancet.h"

#define TITLE "ManCet"

#define BLACK 0xFF000000
#define BLUE  0xFF0000FF

#define DEFAULT_SCALE  1000
#define DEFAULT_OFFSET_X 0
#define DEFAULT_OFFSET_Y 0
#define DEFAULT_ITERATIONS 100
#define DEFAULT_ZOOM_FACTOR 1.1

void mancet_run(uint32_t window_width, uint32_t window_height, uint32_t pixel_width)
{
	/* INIT */

	const uint32_t width  = window_width  / pixel_width;
	const uint32_t height = window_height / pixel_width;

	bool quit = false;

	bool redraw      = true;
	uint32_t *pixels = calloc(width * height, sizeof *pixels);

	bool replot         = true;
	float scale         = DEFAULT_SCALE;
	uint32_t iterations = DEFAULT_ITERATIONS;
	vec2 size_half      = { width / 2.0, height / 2.0 };

	SDL_Init(SDL_INIT_VIDEO);

	SDL_Window *window = SDL_CreateWindow(
		TITLE,
		SDL_WINDOWPOS_CENTERED_DISPLAY(0),
		SDL_WINDOWPOS_CENTERED_DISPLAY(0),
		width,
		height,
		SDL_WINDOW_ALLOW_HIGHDPI
	);

	SDL_Renderer *renderer = SDL_CreateRenderer(
		window,
		-1,
		SDL_RENDERER_PRESENTVSYNC
	);

	SDL_Texture *screen = SDL_CreateTexture(
		renderer,
		SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		width,
		height
	);

	const uint32_t ticks_per_second = 60;
	float time_step_scaled = (1.0 / ticks_per_second) * SDL_GetPerformanceFrequency();

	float current_time = SDL_GetPerformanceCounter();
	float new_time;
	float accumulator = 0;

	while (!quit) {
		/* UPDATE */

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT:
					quit = true;
					break;
			}
		}

		const uint8_t *keys = SDL_GetKeyboardState(NULL);
		if (keys[SDL_SCANCODE_ESCAPE])
			quit = true;

		if (keys[SDL_SCANCODE_UP]) {
			scale *= DEFAULT_ZOOM_FACTOR;
			// iterations *= DEFAULT_ZOOM_FACTOR;

			replot = true;
		}
		if (keys[SDL_SCANCODE_DOWN]) {
			scale /= DEFAULT_ZOOM_FACTOR;

			replot = true;
		}

		new_time = SDL_GetPerformanceCounter();

		accumulator += new_time - current_time;
		current_time = new_time;

		while (accumulator >= time_step_scaled) {
			accumulator -= time_step_scaled;
			
			/* TICK */
			if (!replot)
				continue;

			redraw = true;
			replot = false;

			for (uint32_t y = 0; y < height; ++y) {
				for (uint32_t x = 0; x < width; ++x) {
					vec2 c = { (x - size_half.x) / scale, (y - size_half.y) / scale };

					double rate_of_change = 0.0;

					vec2 z0 = { 0.0, 0.0 }, z1;
					for (uint32_t i = 0; i < iterations; ++i) {
						float temp = comp_abs_no_sqrt(z0);

						// Z(n) = Z(n-1)^2 + c
						z1 = comp_add(comp_square(z0), c);
						z0 = z1;

						rate_of_change += comp_abs_no_sqrt(z1) - temp;
					}

					uint32_t color;

					if (fabs(rate_of_change) > 0.0000000001)
						color = BLACK;
					else
						color = BLUE;

					pixels[x + y * width] = color;
				}
			}
		}

		/* RENDER */

		if (redraw) {
			redraw = false;

			SDL_UpdateTexture(screen, NULL, pixels, width * sizeof(*pixels));
		}

		SDL_RenderCopyEx(renderer, screen, NULL, NULL, 0.0, NULL, SDL_FLIP_VERTICAL);
		SDL_RenderPresent(renderer);
	}

	
	/* QUIT */

	free(pixels);

	SDL_DestroyTexture(screen);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();
}
