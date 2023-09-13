#include "process.h"
#include <stdint.h>
#include <stdio.h>

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))
#define swap(a, b) {void* swap_temp = a; a = b; b = swap_temp;}

typedef struct {
	uint32_t count;
	struct {
		int16_t x;
		int16_t y;
	}
	p[952 * 952 / 4];
} PixelList;

void filter(GrayScale* output, GrayScale* input, PixelList* pixels, uint8_t thold);
void remove_black_pixels(GrayScale* image, PixelList* pixels);
void erode_pass(GrayScale* output, GrayScale* input, PixelList* pixels);
void detect_pass(BitmapData* outbmp, GrayScale* image, PixelList* pixels);

uint64_t sum_pixels(GrayScale* image)
{
	uint64_t sum = 0;
	for (uint32_t i = 0; i < image->width * image->height; i++)
	{
		sum += image->data[i];
	}
	return sum;
}

void mark_cells(BitmapData* outbmp)
{
	GrayScale b0, b1;
	GrayScale* image = &b0;
	GrayScale* buffer = &b1;

	bmp_to_grayscale(outbmp, image);
	init_gray(buffer, b0.width, b0.height);

	PixelList whites;
	whites.count = 0;
	filter(image, image, &whites, 88);
	
	printf("%u whites in total\n", whites.count);

	while (whites.count) {
		detect_pass(outbmp, image, &whites);
		erode_pass(buffer, image, &whites);

		swap(image, buffer);

		remove_black_pixels(image, &whites);

		printf("%u whites remain\n", whites.count);
	}
}

void filter(GrayScale* output, GrayScale* input, PixelList* pixels, uint8_t thold)
{
	for (uint32_t y = 0; y < input->height; y++)
	{
		uint32_t offset = y * input->width;
		for (uint32_t x = 0; x < input->height; x++)
		{
			uint32_t index = offset + x;
			if (input->data[index] > thold) {
				output->data[index] = 255;
				pixels->p[pixels->count].x = x;
				pixels->p[pixels->count].y = y;
				pixels->count++;
			} else {
				output->data[index] = 0;
			}
		}
	}
}

void remove_black_pixels(GrayScale* image, PixelList* pixels)
{
	uint32_t write = 0;
	for (uint32_t read = 0; read < pixels->count; read++)
	{
		int16_t x = pixels->p[read].x;
		int16_t y = pixels->p[read].y;
		if (!image->data[y * image->width + x]) continue;

		pixels->p[write].x = x;
		pixels->p[write].y = y;
		write++; 
	}
	pixels->count = write;
}

__attribute__((always_inline)) inline uint32_t get_cell(GrayScale* image, uint32_t x, uint32_t y)
{
	if (x >= image->width) return 0;
	if (y >= image->height) return 0;
	return image->data[y * image->width + x];
}

void erode_pass(GrayScale* output, GrayScale* input, PixelList* pixels)
{
	uint32_t width = output->width;
	for (uint32_t i = 0; i < pixels->count; i++)
	{
		int32_t x = pixels->p[i].x;
		int32_t y = pixels->p[i].y;

		output->data[y * width + x] = get_cell(input, x, y)
									& get_cell(input, x - 1, y)
									& get_cell(input, x + 1, y)
									& get_cell(input, x, y - 1)
									& get_cell(input, x, y + 1);
	}
}

#define DETECT_SIZE 7
void detect_cell(GrayScale* input,int32_t cx, int32_t cy);

void detect_pass(BitmapData* outbmp, GrayScale* image, PixelList* pixels)
{
	for (uint32_t i = 0; i < pixels->count; i++)
	{
		int32_t x = pixels->p[i].x;
		int32_t y = pixels->p[i].y;

		detect_cell(image, x, y);
	}
}

void detect_cell(GrayScale* image, int32_t cx, int32_t cy)
{
	{
		int32_t minx = max(0, cx - (DETECT_SIZE /  2 + 1));
		int32_t maxx = min((int32_t)image->width, cx + (DETECT_SIZE / 2 + 1) + 1);

		int32_t miny = max(0, cy - (DETECT_SIZE /  2 + 1));
		int32_t maxy = min((int32_t)image->height, cy + (DETECT_SIZE / 2 + 1) + 1);

		uint32_t exclude = 0;
		for (int32_t x = minx; x < maxx; x++)
		{
			exclude += image->data[miny * image->width + x];
			exclude += image->data[maxy * image->width + x];
		}
		if (exclude) return;
		for (int32_t y = miny; y < maxy; y++)
		{
			exclude += image->data[y * image->width + minx];
			exclude += image->data[y * image->width + maxx];
		}
		if (exclude) return;
	}

	{
		int32_t minx = max(0, cx - (DETECT_SIZE /  2));
		int32_t maxx = min((int32_t)image->width, cx + (DETECT_SIZE / 2) + 1);

		int32_t miny = max(0, cy - (DETECT_SIZE /  2));
		int32_t maxy = min((int32_t)image->height, cy + (DETECT_SIZE / 2) + 1);

		uint32_t found = 0;
		for (int32_t y = miny; y < maxy; y++)
		{
			for (int32_t x = minx; x < maxx; x++)
			{
				found += image->data[y * image->height + x];
			}	
		}
		if (!found) return;

		for (int32_t y = miny; y < maxy; y++)
		{
			for (int32_t x = minx; x < maxx; x++)
			{
				image->data[y * image->height + x] = 0;
			}
		}
	}
}