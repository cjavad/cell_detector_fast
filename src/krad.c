#include "krad.h"

#include "bitmap.h"
#include "grayscale.h"
#include <stdint.h>

typedef struct {
	uint32_t count;
	struct {
		uint16_t x;
		uint16_t y;
	}
	p[952 * 952 / 4];
} PixelList;

void krad_filter(GrayScale* output, GrayScale* input, PixelList* const pixels, uint8_t thold);
void krad_remove_black_pixels(GrayScale* image, GrayScale* buffer, PixelList* const pixels);
void krad_edge_pass(GrayScale* dest, GrayScale* src, const PixelList* const pixels);

void krad_debug_snapshot(uint32_t id, GrayScale* image, GrayScale* buffer, PixelList* pixels)
{
	BitmapImage bmp;
	create_bitmap(&bmp, 950, 950);

	for (uint32_t y = 0; y < 950; y++)
	{
		uint32_t offsetBMP = y * bmp.bitmap.row_width;
		uint32_t offset = y * 950;
		for (uint32_t x = 0; x < 950; x++)
		{
            bmp_set_offset(
                &bmp.bitmap, 
                offsetBMP + 3 * x,
                0,
                buffer->data[offset + x],
                buffer->data[offset + x]
            );
		}

		for (uint32_t i = 0; i < pixels->count; i++)
		{
			int32_t x = pixels->p[i].x;
			int32_t y = pixels->p[i].y;
			bmp.bitmap.data[y * bmp.bitmap.row_width + x * 3 + 2] = 255;
		}
	}

	char buff[512];
	sprintf(buff, "res/krad%u.bmp", id);

	FILE* fp = fopen(buff, "wb");

	write_bitmap(fp, &bmp);

	fclose(fp);
}

void krad_pass(BitmapData* outbmp)
{
	GrayScale b0, b1;
	GrayScale* image = &b0;
	GrayScale* buffer = &b1;

	bmp_to_grayscale(outbmp, image);
	init_gray(buffer, b0.width, b0.height);

	PixelList whites;
	whites.count = 0;
	krad_filter(image, image, &whites, 88);

	krad_edge_pass(buffer, image, &whites);
	krad_remove_black_pixels(buffer, image, &whites);

	krad_debug_snapshot(0, buffer, image, &whites);
}




void krad_kgen()
{

}

void krad_filter(GrayScale* output, GrayScale* input, PixelList* const pixels, uint8_t thold)
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

void krad_remove_black_pixels(GrayScale* image, GrayScale* buffer, PixelList* const pixels)
{
	uint32_t write = 0;
	for (uint32_t read = 0; read < pixels->count; read++)
	{
		int32_t x = pixels->p[read].x;
		int32_t y = pixels->p[read].y;
		if (image->data[y * image->width + x] == 0) {
			buffer->data[y * image->width + x] = 0;
			continue;
		}

		pixels->p[write].x = x;
		pixels->p[write].y = y;
		write++; 
	}
	pixels->count = write;
}

__attribute__((always_inline)) inline uint32_t getPixel(GrayScale* image, uint32_t x, uint32_t y)
{
	if (__builtin_expect(x >= image->width, 0)) return 0;
	if (__builtin_expect(y >= image->height, 0)) return 0;
	return image->data[y * image->width + x];
}

void krad_edge_pass(GrayScale* dest, GrayScale* src, const PixelList* const pixels)
{
	for (uint32_t i = 0; i < pixels->count; i++)
	{
		uint32_t x = pixels->p[i].x;
		uint32_t y = pixels->p[i].y;

		int32_t average = (
			(
				src->data[y * src->width + x] * 4
			) 
			-
			(
				getPixel(src, x, y - 1) +
				getPixel(src, x - 1, y) +
				getPixel(src, x + 1, y) +
				getPixel(src, x, y + 1)
			)
		);

		dest->data[y * src->height + x] = average < 0 ? 0 : average > 255 ? 255 : average;
	}
}