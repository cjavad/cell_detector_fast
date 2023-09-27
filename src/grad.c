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

	float fac = 1 / sqrt(gx * gx + gy * gy);

	*dx = gx * fac;
	*dy = gy * fac;
}

void gen_grad(Image32f* image, int id)
{
	BitmapImage img;
	init_bitmap(&img, image->width, image->height);

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
		}
	}

	char buff[512];
	sprintf(buff, "res/grad_%i.bmp", id);

	FILE* fp = fopen(buff, "wb");
	write_bitmap(fp, &img);
	fclose(fp);

	free_bitmap(&img);
}

/*
	<x0, y0> + s * <dx0, dy0> == <x1, y1> + t * <dx1, dy1>

	x0 + s * dx0 = x1 + t * dx1
	y0 + s * dy0 = y1 + t * dy1

	s * dx0 = x1 - x0 + t * dx1
	s * dy0 - t * dy1 = y1 - y0

	s = (x1 - x0) / dx0 + t * dx1 / dx0
	(x1 - x0) * dy0 / dx0 + t * dx1 * dy0 / dx0 - t * dy1 = y1 - y0
	t * (dx1 * dy0 / dx0 - dy1) = y1 - y0 - (x1 - x0) * dy0 / dx0

	t = (y1 - y0 - (x1 - x0) * dy0 / dx0) / (dx1 * dy0 / dx0 - dy1)
	

*/

inline void find_intersect(float* o_s, float* o_t, grad_point_t* p0, grad_point_t* p1)
{
	float dx = p1->x - p0->x;
	float dy = p1->y - p0->y;

	float g0 = p0->dy / p0->dx;

	float t = (dy - dx * g0) / (p1->dx * g0 - p1->dy);
	float s = (dx + t * p1->dx) / p0->dx;

	*o_s = s;
	*o_t = t;
}

#define STEP_A 8
#define STEP_B 2

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

void grad_pass(BitmapData* input, int id)
{
	uint32_t thold = 88;

	Image8u img;
	init_image8u(&img, input->width, input->height, 32);

	point_list_t points;
	vec_init(&points);

	// find edge points and calculate gradient
	for (uint32_t y = 0; y < input->height; y++)
	{
		for (uint32_t x = 0; x < input->width; x++)
		{
			if (bmp_get_pixel(input, x, y, 0) <= thold) continue;

			uint8_t xm1 = bmp_get_pixel_secure(input, x - 1, y, 0);
			uint8_t xp1 = bmp_get_pixel_secure(input, x + 1, y, 0);
			uint8_t ym1 = bmp_get_pixel_secure(input, x, y - 1, 0);
			uint8_t yp1 = bmp_get_pixel_secure(input, x, y + 1, 0);

			if (
				xm1 <= thold ||
				xp1 <= thold ||
				ym1 <= thold ||
				yp1 <= thold
			) continue;

			image8u_set_pixel(&img, x, y, 255);

			vec_push(&points, ((point_t){x, y}));
		}
	}

	point_list_t results;
	vec_init(&results);

	point_list_t fill;
	vec_init(&fill);

	grad_point_list_t set;
	vec_init(&set);

	for (uint32_t i = 0; i < points.len; i++)
	{
		point_t pt = points.data[i];
		// black check
		if (!image8u_get_pixel(&img, pt.x, pt.y)) continue;
		
		// flood fill

		set.len = 0;
		fill.len = 0;

		vec_push(&fill, pt);

		while (fill.len) {
			pt = vec_pop(&fill);
			grad_point_t p;
			find_grad(&p, input, pt.x, pt.y);

			vec_push(&set, p);
			image8u_set_pixel(&img, p.x, p.y, 0);

			uint8_t mx = image8u_get_pixel(&img, p.x - 1, p.y);
			uint8_t px = image8u_get_pixel(&img, p.x + 1, p.y);
			uint8_t my = image8u_get_pixel(&img, p.x, p.y - 1);
			uint8_t py = image8u_get_pixel(&img, p.x, p.y + 1);

			if (mx) vec_push(&fill, ((point_t){p.x - 1, p.y}));
			if (px) vec_push(&fill, ((point_t){p.x + 1, p.y}));
			if (my) vec_push(&fill, ((point_t){p.x, p.y - 1}));
			if (py) vec_push(&fill, ((point_t){p.x, p.y + 1}));
		}

		for (uint32_t j = 0; j < set.len; j += STEP_A)
		{
			grad_point_t o = set.data[j];

			uint32_t cx = o.x;
			uint32_t cy = o.y;
			uint32_t pcount = 1; 

			for (uint32_t k = 0; k < set.len; k += STEP_B) 
			{
				if (k == j) continue;
				grad_point_t p = set.data[k];

				int32_t dx = p.x - o.x;
				int32_t dy = p.y - o.y;

				if (dx * dx + dy * dy > 24 * 24) continue;

				float s, t;
				find_intersect(&s, &t, &o, &p);
				
				if (s < 0 || t < 0) continue;
				
				float idx = s * o.dx;
				float idy = s * o.dy;

				if (idx * idx + idy * idy > 12 * 12) continue;

				cx += p.x;
				cy += p.y;
				pcount++;
			}

			if (pcount <= 15) continue;
			vec_push(&results, ((point_t){(cx / pcount),(cy / pcount)}));
		}
	}

	vec_free(&fill);
	vec_free(&set);

	BitmapImage output;
	init_bitmap(&output, input->width, input->height);
	memcpy(output.bitmap.data, input->data, input->row_width * input->height);

	for (uint32_t i = 0; i < results.len; i++)
	{
		point_t p = results.data[i];
		draw_cross(&output.bitmap, p.x, p.y, 255, 0, 0);
	}

	char buff[512];
	sprintf(buff, "res/grad_%i.bmp", id);

	FILE* fp = fopen(buff, "wb");
	write_bitmap(fp, &output);
	fclose(fp);

	destroy_image8u(&img);
	free_bitmap(&output);

	vec_free(&results);
	vec_free(&points);
}