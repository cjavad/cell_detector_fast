#include <stdint.h>

#include <stdio.h>
#include <stdlib.h>

#include "bitmap.h"
#include "grayscale.h"

void blur_pass(GrayScale* dest, GrayScale* src);
void range_pass(GrayScale* dest, GrayScale* src, uint8_t min, uint8_t max);
void edge_pass(GrayScale* dest, GrayScale* src);

int32_t main()
{
    FILE* fp = fopen("res/example.bmp", "rb");

	BitmapHeader header;
	BitmapInfoHeader infoHeader;
	Bitmap bitmap;

	read_bitmap(fp, &header, &infoHeader, &bitmap);

    fclose(fp);

	printf("Magic = %.2s\n", (char *) &header.magic);
	printf("Size = %ix%i\n", infoHeader.width, infoHeader.height);
	printf("Bits per pixel = %u\n", infoHeader.bpp);

    fp = fopen("res/out.bmp", "wb");

    GrayScale image;
    bmp_to_grayscale(&bitmap, &image);

	GrayScale output;
	output.width = image.width;
	output.height = image.height;
	output.data = malloc(output.width * output.height * sizeof(uint8_t));
	range_pass(&image, &image, 64, 255);
	range_pass(&image, &image, 32, 33);
	edge_pass(&output, &image);

    write_grayscale(fp, &output);

    fclose(fp);

	return 0;
}

uint32_t getPixel(GrayScale* image, int64_t x, int64_t y)
{
	if (x < 0 || x >= image->width) return 0;
	if (y < 0 || y >= image->height) return 0;
	return image->data[y * image->width + x];
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
			uint32_t average = ( 
					getPixel(src, x - 1, y - 1) * 0 + getPixel(src, x, y - 1) * 1 + getPixel(src, x + 1, y - 1) * 0  +
					getPixel(src, x - 1, y)     * 1 + getPixel(src, x, y)     * -4 + getPixel(src, x + 1, y)     * 1  +
					getPixel(src, x - 1, y + 1) * 0 + getPixel(src, x, y + 1) * 1 + getPixel(src, x + 1, y + 1) * 0
			);
			dest->data[offset + x] = average;
		}
	}
}
