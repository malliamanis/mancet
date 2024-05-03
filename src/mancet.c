#include <SDL2/SDL.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "mancet.h"

#define TITLE "cgraph"

void mancet_run(uint32_t window_width, uint32_t window_height, uint32_t pixel_width)
{
	/* INIT */

	const uint32_t width  = window_width  / pixel_width;
	const uint32_t height = window_height / pixel_width;

	bool quit = false;

	uint32_t *pixels = calloc(width * height, sizeof *pixels);
	bool redraw = true;

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
	double time_step_scaled = (1.0 / ticks_per_second) * SDL_GetPerformanceFrequency();

	double current_time = SDL_GetPerformanceCounter();
	double new_time;
	double accumulator = 0;

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

		new_time = SDL_GetPerformanceCounter();

		accumulator += new_time - current_time;
		current_time = new_time;

		while (accumulator >= time_step_scaled) {
			accumulator -= time_step_scaled;
			
			/* TICK */

		}

		/* RENDER */

		if (redraw) {
			redraw = false;

			memset(pixels, 0xFF, width * height * sizeof(*pixels));

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
