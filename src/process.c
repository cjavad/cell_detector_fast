#include "process.h"

#define max(a, b) (a > b ? a : b)
#define min(a, b) (a < b ? a : b)

typedef struct {
	uint32_t count;
	struct {
		int16_t x;
		int16_t y;
	}
	p[952 * 952 / 4];
} PixelList;

void filter(GrayScale* image, PixelList* pixels, uint8_t thold);
void remove_blacks(GrayScale* image, PixelList* pixels);
void erode_pass(GrayScale* image, uint);
uint32_t detect_pass();

void mark_cells(BitmapData* outbmp, GrayScale* input)
{
	GrayScale b0, b1;
	

	PixelList whites;
	whites.count = 0;
	filter(input, &whites, 88);
	
	printf("%u whites in total\n", whites.count);

	while (whites.count) {

		remove_blacks()
	}
}

void filter(GrayScale* image, PixelList* pixels, uint8_t thold)
{
	for (uint32_t y = 0; y < image->height; y++)
	{
		uint32_t offset = y * image->width;
		for (uint32_t x = 0; x < image->height; x++)
		{
			uint32_t index = offset + x;
			if (image->data[index] > thold) {
				image->data[index] = 255;
				pixels->p[pixels->count].x = x;
				pixels->p[pixels->count].y = y;
				pixels->count++;
			} else {
				image->data[index] = 255;
			}
		}
	}
}

void remove_blacks(GrayScale* image, PixelList* pixels)
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