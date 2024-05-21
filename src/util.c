#include "math.h"

#include "util.h"

uint32_t util_hsv_to_rgb(float hue, float saturation, float value)
{
	const float hue_norm = hue / 60.0f;
	const float saturation_shift = 1.0f - saturation;

	const float gradient_inc = fmodf(hue_norm, 1.0f);
	const float gradient_dec = 1.0f - gradient_inc;

	float r = 0.0f, g = 0.0f, b = 0.0f;
	switch ((uint32_t) hue_norm) {
		case 0:
			r = 1.0f;
			g = gradient_inc * saturation + saturation_shift;
			b = saturation_shift;
			break;
		case 1:
			r = gradient_dec * saturation + saturation_shift;
			g = 1.0f;
			b = saturation_shift;
			break;
		case 2:
			r = saturation_shift;
			g = 1.0f;
			b = gradient_inc * saturation + saturation_shift;
			break;
		case 3:
			r = saturation_shift;
			g = gradient_dec * saturation + saturation_shift;
			b = 1.0f;
			break;
		case 4:
			r = gradient_inc * saturation + saturation_shift;
			g = saturation_shift;
			b = 1.0f;
			break;
		case 5:
			r = 1;
			g = saturation_shift;
			b = gradient_dec * saturation + saturation_shift;
			break;
		default:
			break;
	}

	uint32_t rgb_color = 0x00;
	rgb_color += (uint32_t)(r * value * 0xFF) << 16;
	rgb_color += (uint32_t)(g * value * 0xFF) << 8;
	rgb_color += (uint32_t)(b * value * 0xFF);

	return rgb_color;
}
