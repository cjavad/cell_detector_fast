#include "grad.h"
#include <stdint.h>


#include <math.h>

#include <stdio.h>

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