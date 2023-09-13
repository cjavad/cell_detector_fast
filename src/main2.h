#include <stdint.h>

#include <stdio.h>
#include <stdlib.h>

#include "bitmap.h"
#include "grayscale.h"
#include "erode.h"
#include "process.h"

void blur_pass(GrayScale* dest, GrayScale* src);
void range_pass(GrayScale* dest, GrayScale* src, uint8_t min, uint8_t max);
void edge_pass(GrayScale* dest, GrayScale* src);
void bonc_pass(GrayScale* dest, GrayScale* src, uint32_t thresh, GrayScale* g);


void erode(GrayScale* dest, GrayScale* src);

int32_t main()
{
	FILE* fp = fopen("res/example.bmp", "rb");

	BitmapHeader      header;
	BitmapInfoHeader  infoHeader;
	BitmapData            data;

    BitmapImage bitmap;

    bitmap.bitmap = &data;
    bitmap.header = &header;
    bitmap.infoHeader = &infoHeader;

	read_bitmap(fp, &bitmap);

	fclose(fp);

	printf("Magic          = %.2s\n", (char *) &header.magic);
	printf("Size           = %ix%i\n", infoHeader.width, infoHeader.height);
	printf("Bits per pixel = %u\n", infoHeader.bpp);

	fp = fopen("res/out.bmp", "wb");

	GrayScale image;
	bmp_to_grayscale(&data, &image);

	goto there;

	GrayScale output;
	init_gray(&output, 950, 950);
	GrayScale outpt[16];
	for (uint16_t i = 0; i < 16; i++) init_gray(&outpt[i], 950, 950);
	// blur_pass(&output, &image);
	// range_pass(&output, &output, 64, 192);
	// blur_pass(&image, &output);
	range_pass(&outpt[0], &image, 64, 255);
	range_pass(&outpt[1], &outpt[0], 32, 33);
	edge_pass(&outpt[2], &outpt[1]);

	// write_grayscale(fp, &output);

	uint32_t thresh = 176;

	// memcpy(outpt[3].data, outpt[1].data, 950 * 950);
	// bonc_pass(&outpt[3], &outpt[2], thresh, &outpt[1]);
	// edge_pass(&outpt[2], &outpt[3]);
	// bonc_pass(&outpt[1], &outpt[2], thresh, &outpt[3]);
	// edge_pass(&outpt[2], &outpt[1]);
	// bonc_pass(&outpt[3], &outpt[2], thresh, &outpt[1]);
	// edge_pass(&outpt[2], &outpt[3]);
	// bonc_pass(&outpt[1], &outpt[2], thresh, &outpt[3]);
	// edge_pass(&outpt[2], &outpt[1]);
	// bonc_pass(&outpt[3], &outpt[2], thresh, &outpt[1]);
	// edge_pass(&outpt[2], &outpt[3]);
	// bonc_pass(&outpt[1], &outpt[2], thresh, &outpt[3]);
	// edge_pass(&outpt[2], &outpt[1]);
	// bonc_pass(&outpt[3], &outpt[2], thresh, &outpt[1]);

	// erode(&outpt[2], &outpt[1]);
	// erode(&outpt[1], &outpt[2]);
	
	erode_cells(&outpt[2], &outpt[1], &outpt[3]);
	
	for (uint32_t y = 0; y < data.height; y++)
	{
		uint32_t offsetB = y * data.row_width;
		uint32_t offsetI = y * data.width;
		for (uint32_t x = 0; x < data.width; x++)
		{
			data.data[offsetB + x * data.byte_pp + 0] = outpt[0].data[offsetI + x];//outpt[0].data[offsetI + x];
			data.data[offsetB + x * data.byte_pp + 1] = outpt[0].data[offsetI + x];
			data.data[offsetB + x * data.byte_pp + 2] = outpt[3].data[offsetI + x];
		}
	}



	write_bitmap(fp, &bitmap);

	there:

    fclose(fp);

	mark_cells(NULL, &image);

	return 0;
}

uint32_t getPixel(GrayScale* image, int64_t x, int64_t y)
{
	if (x < 0 || x >= image->width) return 0;
	if (y < 0 || y >= image->height) return 0;
	return image->data[y * image->width + x];
}

void erode(GrayScale* dest, GrayScale* src)
{
	for (uint32_t y = 0; y < src->height; y++)
	{
		uint32_t offset = y * src->height;
		for (uint32_t x = 0; x < src->width; x++)
		{
			uint32_t average = ( 
					getPixel(src, x, y - 1) &
					getPixel(src, x - 1, y) & getPixel(src, x, y) & getPixel(src, x + 1, y) &
					getPixel(src, x, y + 1)
			);
			dest->data[offset + x] = average;
		}
	}
}

void blur_pass(GrayScale* dest, GrayScale* src)
{
	for (uint32_t y = 0; y < src->height; y++)
	{
		uint32_t offset = y * src->height;
		for (uint32_t x = 0; x < src->width; x++)
		{
			uint32_t average = ( 
					getPixel(src, x - 1, y - 1)     + getPixel(src, x, y - 1) * 2 + getPixel(src, x + 1, y - 1)       +
					getPixel(src, x - 1, y)     * 2 + getPixel(src, x, y)     * 4 + getPixel(src, x + 1, y)      * 2  +
					getPixel(src, x - 1, y + 1)     + getPixel(src, x, y + 1) * 2 + getPixel(src, x + 1, y + 1)
			) / 16;
			dest->data[offset + x] = average;
		}
	}
}

void range_pass(GrayScale* dest, GrayScale* src, uint8_t min, uint8_t max)
{
	float mul = 255.0f / (float)(max - min);
	for (uint32_t y = 0; y < src->height; y++)
	{
		uint32_t offset = y * src->height;
		for (uint32_t x = 0; x < src->width; x++)
		{
			float val = src->data[offset + x] - min;
			val *= mul;
			val = val < 0 ? 0 : val > 255 ? 255 : val;
			dest->data[offset + x] = (uint8_t) val;
		}
	}
}

void edge_pass(GrayScale* dest, GrayScale* src)
{
	for (uint32_t y = 0; y < src->height; y++)
	{
		uint32_t offset = y * src->height;
		for (uint32_t x = 0; x < src->width; x++)
		{
			int32_t average = ( 
					getPixel(src, x - 1, y - 1) *  0 + getPixel(src, x, y - 1) * -1 + getPixel(src, x + 1, y - 1) *  0  +
					getPixel(src, x - 1, y)     * -1 + getPixel(src, x, y)     *  4 + getPixel(src, x + 1, y)      * -1  +
					getPixel(src, x - 1, y + 1) *  0 + getPixel(src, x, y + 1) * -1 + getPixel(src, x + 1, y + 1) *  0
			);
			dest->data[offset + x] = average < 0 ? 0 : average > 255 ? 255 : average;
		}
	}
}

uint32_t bonc_check(uint32_t x, uint32_t y, GrayScale* g)
{
	uint32_t social_credit_score = 0;
	for (int32_t i = -3; i <= 3; i++)
	{
		for (int32_t j = -3; j <= 3; j++)
		{
			social_credit_score += getPixel(g, x + j, y + i);
		}
	}
	return social_credit_score;
}

void bonc_pass(GrayScale* dest, GrayScale* src, uint32_t thresh, GrayScale* g)
{
	for (uint32_t y = 0; y < src->height; y++)
	{
		uint32_t offset = y * src->height;
		for (uint32_t x = 0; x < src->width; x++)
		{
			if (__builtin_expect(!src->data[offset + x], 0)) continue;

			uint32_t check = bonc_check(x, y, g);
			if (check / 49 > thresh) {
				dest->data[offset + x] = 0;
			}
		}
	}
}
