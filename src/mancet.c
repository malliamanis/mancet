#include <SDL2/SDL.h>

#include <SDL2/SDL_mouse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "comp.h"
#include "mancet.h"

#define TITLE "ManCet"

#define DEFAULT_SCALE  100
#define DEFAULT_OFFSET_X 0
#define DEFAULT_OFFSET_Y 0
#define DEFAULT_ITERATIONS 50
#define DEFAULT_ITERATIONS_INCREASE 250
#define DEFAULT_ZOOM_FACTOR 1.5

void mancet_run(uint32_t window_width, uint32_t window_height, uint32_t pixel_width)
{
	/* INIT */

	const uint32_t width  = window_width  / pixel_width;
	const uint32_t height = window_height / pixel_width;

	bool quit = false;

	bool redraw      = true;
	uint32_t *pixels = calloc(width * height, sizeof *pixels);

	bool replot         = true;
	vec2 size_half      = { width / 2.0, height / 2.0 };
	double scale         = DEFAULT_SCALE;
	vec2 offset         = { 0.0f, 0.0f };
	uint32_t iterations = DEFAULT_ITERATIONS;
	bool iterations_changed = true;

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

	int32_t mouse_last_x, mouse_last_y;
	int32_t mouse_scroll_amount;

	const uint32_t y_render_freq = height >> 3;

	const uint32_t ticks_per_second = 60;
	double time_step_scaled = (1.0 / ticks_per_second) * SDL_GetPerformanceFrequency();

	double current_time = SDL_GetPerformanceCounter();
	double new_time;
	double accumulator = 0;

	while (!quit) {
		/* UPDATE */

		mouse_scroll_amount = 0;

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT:
					quit = true;
					break;
				case SDL_MOUSEBUTTONDOWN:
					if (event.button.button == SDL_BUTTON_LEFT) {
						mouse_last_x = event.button.x;
						mouse_last_y = event.button.y;
					}
					break;
				case SDL_MOUSEBUTTONUP:
					if (event.button.button == SDL_BUTTON_LEFT) {
						offset.x += event.button.x - mouse_last_x;
						offset.y += event.button.y - mouse_last_y;

						replot = true;
					}
					break;
				case SDL_MOUSEWHEEL:
					mouse_scroll_amount = event.wheel.y;
					break;
			}
		}

		const uint8_t *keys = SDL_GetKeyboardState(NULL);
		if (keys[SDL_SCANCODE_ESCAPE])
			quit = true;

		if (mouse_scroll_amount > 0) {
			scale *= DEFAULT_ZOOM_FACTOR;

			offset.x *= DEFAULT_ZOOM_FACTOR;
			offset.y *= DEFAULT_ZOOM_FACTOR;

			replot = true;
		}
		else if (mouse_scroll_amount < 0) {
			scale /= DEFAULT_ZOOM_FACTOR;

			offset.x /= DEFAULT_ZOOM_FACTOR;
			offset.y /= DEFAULT_ZOOM_FACTOR;

			if (iterations == 0)
				iterations = 1;

			replot = true;
		}

		if (keys[SDL_SCANCODE_UP]) {
			iterations += DEFAULT_ITERATIONS_INCREASE;
			iterations_changed = true;
			replot = true;
		}
		if (keys[SDL_SCANCODE_DOWN] && iterations > DEFAULT_ITERATIONS) {
			iterations -= DEFAULT_ITERATIONS_INCREASE;
			iterations_changed = true;
			replot = true;
		}

		if (iterations_changed) {
			iterations_changed = false;

			printf("\33[2K\r");
			printf("ITERATIONS: %d", iterations);
			fflush(stdout);
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
					vec2 c = {
						(x - offset.x - size_half.x) / scale,
						(y + offset.y - size_half.y) / scale 
					};

					uint32_t color;
					double rate_of_change = 0.0;

					vec2 z0 = { 0.0, 0.0 }, z1;
					for (uint32_t i = 0; i < iterations; ++i) {
						// Z(n) = Z(n-1)^2 + c
						z1 = comp_add(comp_square(z0), c);
						z0 = z1;

						rate_of_change += z1.x * z1.x + z1.y * z1.y;

						if (isnan(rate_of_change) || isinf(rate_of_change)) {
							color = 0xFFFFFFFF;
							goto skip_black;
						}
					}

					color =  0x00;

					skip_black:
					
					pixels[x + y * width] = color;
				}

				if (y % y_render_freq == 0) {
					SDL_UpdateTexture(screen, NULL, pixels, width * sizeof(*pixels));

					SDL_RenderCopyEx(renderer, screen, NULL, NULL, 0.0, NULL, SDL_FLIP_VERTICAL);
					SDL_RenderPresent(renderer);
				}
			}
		}

		/* RENDER */

		if (redraw) {
			redraw = false;
			SDL_UpdateTexture(screen, NULL, pixels, width * sizeof(*pixels));

			memset(pixels, 0x00, width * height * sizeof(*pixels));
		}

		SDL_RenderCopyEx(renderer, screen, NULL, NULL, 0.0, NULL, SDL_FLIP_VERTICAL);
		SDL_RenderPresent(renderer);
	}

	putchar('\n');
	
	/* QUIT */

	free(pixels);

	SDL_DestroyTexture(screen);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();
}
