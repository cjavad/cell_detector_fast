#include "grad.h"
#include <stdint.h>

#include <stdio.h>
#include <string.h>

#include <math.h>

#include "bitmap.h"

void calc_grad(float* dx, float* dy, Image32f* image, int32_t x, int32_t y)
{
	float p = image32f_get_pixel(image, x, y);

	// from left to right (white left = -1, white right = 1, no net = 0)	
	float lr = (image32f_get_pixel(image, x + 1, y) - p) + (p - image32f_get_pixel(image, x - 1, y));
	// same but bottom to top
	float bt = (image32f_get_pixel(image, x, y + 1) - p) + (p - image32f_get_pixel(image, x, y - 1));
	// same but bottom left to top right
	float tr = (image32f_get_pixel(image, x + 1, y + 1) - p) + (p - image32f_get_pixel(image, x - 1, y - 1));
	// same but bottom right to top left
	float tl = (image32f_get_pixel(image, x - 1, y + 1) - p) + (p - image32f_get_pixel(image, x + 1, y - 1));

	float gx = lr + tr * 0.707f - tl * 0.707f;
	float gy = bt + tr * 0.707f + tl * 0.707f;

	float fac = 1;// / sqrt(gx * gx + gy * gy);

	*dx = gx * fac;
	*dy = gy * fac;
}


void gen_grad(Image32f* image, point_list_t* cells)
{
	BitmapImage img;
	init_bitmap(&img, image->width, image->height);

	point_list_t points;
	vec_init(&points);

	grad_point_list_t gpoints;
	vec_init(&gpoints);

	for (uint32_t y = 0; y < image->height; y++)
	{
		for (uint32_t x = 0; x < image->width; x++)
		{
			if (image32f_get_pixel(image, x, y) < 88.0f / 255.0f) {
				bmp_set_pixels(&img.bitmap, x, y, 0, 0, 0);
				continue;
			}
			float dx, dy;
			calc_grad(&dx, &dy, image, x, y);
			float r = ((dx * 127.0f) + 128.0f);
			float g = ((dy * 127.0f) + 128.0f);
			bmp_set_pixels(&img.bitmap, x, y, (uint8_t)r, (uint8_t)g, 0);

			vec_push(&points, ((point_t){x, y}));
			vec_push(&gpoints, ((grad_point_t){x, y, dx, dy}));
		}
	}

	Image8u mask;
	init_image8u(&mask, image->width, image->height, 32);

	#define CUT 0.08f
	#define RADIUS 16
	#define DENIAL 4

	for (uint32_t i = 0; i < gpoints.len; i++)
	{
		grad_point_t gp = gpoints.data[i];

		if (fabs(gp.dx) >= CUT || fabs(gp.dy) >= CUT) continue;

		if (image8u_get_pixel(&mask, gp.x, gp.y)) continue;

		uint32_t fail = 0;

		for (int32_t y = -DENIAL; y < DENIAL + 1; y++)
		{
			for (int32_t x = -DENIAL; x < DENIAL + 1; x++)
			{
				if (x * x + y * y >= DENIAL * DENIAL) continue;

				if (gp.x + x < 0 || gp.x + x >= img.bitmap.width) continue;
				if (gp.y + y < 0 || gp.y + y >= img.bitmap.height) continue;

				if (
					!bmp_get_pixel(&img.bitmap, gp.x + x, gp.y + y, 0) &&
					!bmp_get_pixel(&img.bitmap, gp.x + x, gp.y + y, 1) &&
					!bmp_get_pixel(&img.bitmap, gp.x + x, gp.y + y, 2)
				) {
					fail = 1;
				}
			}
		}

		if (fail) continue;

		vec_push(cells, ((point_t){gp.x, gp.y}));

		for (int32_t y = -RADIUS; y < RADIUS + 1; y++)
		{
			for (int32_t x = -RADIUS; x < RADIUS + 1; x++)
			{
				if (x * x + y * y >= RADIUS * RADIUS) continue;
				image8u_set_pixel(&mask, gp.x + x, gp.y + y, 255);
			}
		}
	}

	free_bitmap(&img);
}

void find_grad(grad_point_t* p, BitmapData* input, int32_t x, int32_t y)
{
	uint8_t xm1 = bmp_get_pixel_secure(input, x - 1, y, 0);
	uint8_t xp1 = bmp_get_pixel_secure(input, x + 1, y, 0);
	uint8_t ym1 = bmp_get_pixel_secure(input, x, y - 1, 0);
	uint8_t yp1 = bmp_get_pixel_secure(input, x, y + 1, 0);

	uint8_t xm1ym1 = bmp_get_pixel_secure(input, x - 1, y - 1, 0);
	uint8_t xp1yp1 = bmp_get_pixel_secure(input, x + 1, y + 1, 0);
	uint8_t xm1yp1 = bmp_get_pixel_secure(input, x - 1, y + 1, 0);
	uint8_t xp1ym1 = bmp_get_pixel_secure(input, x + 1, y - 1, 0);

	// from left to right (white left = -1, white right = 1, no net = 0)	
	float lr = (xp1 - xm1);
	// same but bottom to top
	float bt = (yp1 - ym1);
	// same but bottom left to top right
	float tr = (xp1yp1 - xm1ym1);
	// same but bottom right to top left
	float tl = (xm1yp1 - xp1ym1);

	float gx = lr + tr * 0.707f - tl * 0.707f;
	float gy = bt + tr * 0.707f + tl * 0.707f;

	float fac = 1 / sqrt(gx * gx + gy * gy);

	p->x = x;
	p->y = y;
	
	p->dx = gx * fac;
	p->dy = gy * fac;
}