#include "process.h"
#include "bitmap.h"
#include "grayscale.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>


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
void remove_black_pixels(GrayScale* image, GrayScale* buffer, PixelList* pixels);
void erode_pass(GrayScale* output, GrayScale* input, PixelList* pixels);
void detect_pass(BitmapData* outbmp, GrayScale* image, PixelList* pixels);

#include <stdlib.h>

void debug_snapshot(uint32_t id, GrayScale* image, GrayScale* buffer, PixelList* pixels)
{
	BitmapImage bmp;
	create_bitmap(&bmp, 950, 950);

	for (uint32_t y = 0; y < 950; y++)
	{
		uint32_t offsetBMP = y * bmp.bitmap.row_width;
		uint32_t offset = y * 950;
		for (uint32_t x = 0; x < 950; x++)
		{
			bmp.bitmap.data[offsetBMP + 3 * x + 1] = buffer->data[offset + x];
			bmp.bitmap.data[offsetBMP + 3 * x + 0] = image->data[offset + x];
			bmp.bitmap.data[offsetBMP + 3 * x + 2] = 0;
		}

		for (uint32_t i = 0; i < pixels->count; i++)
		{
			int32_t x = pixels->p[i].x;
			int32_t y = pixels->p[i].y;
			bmp.bitmap.data[y * bmp.bitmap.row_width + x * 3 + 2] = 255;
		}
	}

	char buff[512];
	sprintf(buff, "res/debug%u.bmp", id);

	FILE* fp = fopen(buff, "wb");

	write_bitmap(fp, &bmp);

	fclose(fp);
}

void mark_cells(BitmapData* outbmp)
{
	GrayScale b0, b1;
	GrayScale* image = &b0;
	GrayScale* buffer = &b1;

	bmp_to_grayscale(outbmp, image);
	init_gray(buffer, b0.width, b0.height);

    memset(buffer->data, 0, buffer->width * buffer->height);

	PixelList whites;
	whites.count = 0;

    FILE* pfp = fopen("res/pre-pass0.bmp", "wb");
    write_grayscale(pfp, image);
    fclose(pfp);

	filter(image, image, &whites, 200);

#ifdef DEBUG
	printf("%u whites in total\n", whites.count);

    FILE* fp = fopen("res/pass0.bmp", "wb");
    write_grayscale(fp, image);
    fclose(fp);
    int pc = 1;
#endif

	while (whites.count) {
		// debug_snapshot(pc, image, buffer, &whites);
		detect_pass(outbmp, image, &whites);
		erode_pass(buffer, image, &whites);

#ifdef DEBUG
        char out[512];
        sprintf(out, "res/pass%u.bmp", pc);
        FILE* fp = fopen(out, "wb");
        write_grayscale(fp, buffer);
        fclose(fp);
#endif
		swap(image, buffer);

		remove_black_pixels(image, buffer, &whites);

#ifdef DEBUG
		printf("%u whites remain\n", whites.count);
	    pc++;
#endif
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

void remove_black_pixels(GrayScale* image, GrayScale* buffer, PixelList* pixels)
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

#define DETECT_SIZE 11
#define RSIZE 3

void detect_cell(BitmapData* bmp, GrayScale* input,int32_t cx, int32_t cy);

void detect_pass(BitmapData* outbmp, GrayScale* image, PixelList* pixels)
{
	for (uint32_t i = 0; i < pixels->count; i++)
	{
		int32_t x = pixels->p[i].x;
		int32_t y = pixels->p[i].y;

		detect_cell(outbmp, image, x, y);
	}
}

void detect_cell(BitmapData* bmp, GrayScale* image, int32_t cx, int32_t cy)
{
	// exclusion zone check
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

	// cell check
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
				found += image->data[y * image->width + x];
			}	
		}

		if (!found) return;

		// remove cell
		for (int32_t y = miny; y < maxy; y++)
		{
			for (int32_t x = minx; x < maxx; x++)
			{
				image->data[y * image->width + x] = 0;
			}
		}
	}

	// mark image
    {
        int32_t minx = max(0, cx - RSIZE);
        int32_t maxx = min((int32_t)image->width, cx + RSIZE + 1);

        int32_t miny = max(0, cy - RSIZE);
        int32_t maxy = min((int32_t)image->height, cy + RSIZE + 1);

        for (int32_t y = miny; y < maxy; y++)
        {
            for (int32_t x = minx; x < maxx; x++)
            {
				uint32_t dist = (cy - y) * (cy - y) + (cx - x) * (cx - x);
                // if (dist > RSIZE * RSIZE) continue;
				bmp->data[y * bmp->row_width + (x * 3) + 0] = 0;
				bmp->data[y * bmp->row_width + (x * 3) + 1] = 0;
                bmp->data[y * bmp->row_width + (x * 3) + 2] = 255;
            }
        }
    }
}
