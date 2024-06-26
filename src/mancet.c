#include <SDL2/SDL.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <complex.h>

#include "util.h"
#include "mancet.h"

#define TITLE "ManCet"

#define DEFAULT_SCALE  200
#define DEFAULT_OFFSET_X 0
#define DEFAULT_OFFSET_Y 0
#define DEFAULT_ITERATIONS 100
#define DEFAULT_ITERATIONS_INCREASE 250
#define DEFAULT_ZOOM_FACTOR 1.5

typedef enum {
	COLORSCHEME_HSV = 0,
	COLORSCHEME_GRAYSCALE = 1
} Colorscheme;

void mancet_run(uint32_t window_width, uint32_t window_height, uint32_t pixel_width)
{
	/* INIT */

	const uint32_t width  = window_width  / pixel_width;
	const uint32_t height = window_height / pixel_width;

	bool quit = false;

	bool redraw      = true;
	uint32_t *pixels = calloc(width * height, sizeof *pixels);
	Colorscheme colorscheme = COLORSCHEME_HSV;

	bool replot         = true;
	vec2 size_half      = { width / 2.0, height / 2.0 };
	double scale        = DEFAULT_SCALE;
	vec2 offset         = { 0.0f, 0.0f };
	uint32_t iterations = DEFAULT_ITERATIONS;
	bool iterations_changed = true;

	SDL_Init(SDL_INIT_VIDEO);

	SDL_Window *window = SDL_CreateWindow(
		TITLE,
		SDL_WINDOWPOS_CENTERED_DISPLAY(0),
		SDL_WINDOWPOS_CENTERED_DISPLAY(0),
		window_width,
		window_height,
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
	bool c_key_down = false;

	const uint32_t y_render_freq = height >> 2;

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
						offset.x += (double)(event.button.x - mouse_last_x) / pixel_width;
						offset.y += (double)(event.button.y - mouse_last_y) / pixel_width;

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

		if (keys[SDL_SCANCODE_C]) {
			if (!c_key_down) {
				c_key_down = true;

				colorscheme = !colorscheme;
				replot = true;
			}
		}
		else
			c_key_down = false;

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

		if (keys[SDL_SCANCODE_SPACE]) {
			colorscheme = COLORSCHEME_HSV;
			offset = (vec2) { 0.0f, 0.0f };
			scale = DEFAULT_SCALE;
			iterations = DEFAULT_ITERATIONS;

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
					complex double c = (x - offset.x - size_half.x) + (y + offset.y - size_half.y) * I;
					c /= scale;

					uint32_t i;
					complex double z = c;

					for (i = 0; i < iterations; ++i) {
						z = z*z + c;

						double real = creal(z);
						double imag = cimag(z);

						if (real*real + imag*imag > 4.0)
							break;
					}

					uint32_t color = 0xFF000000;

					if (i < iterations - 1) {
						switch (colorscheme) {
							case COLORSCHEME_HSV:
								color += util_hsv_to_rgb(360.0f * i / iterations, 1.0f, 1.0f);
								break;
							case COLORSCHEME_GRAYSCALE:
								{
									uint32_t brightness = 0xFF * (double)i / iterations;

									color += brightness << 16; // r
									color += brightness << 8;  // g
									color += brightness;       // b
								}
								break;
						}
					}

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
