#include "erode.h"

#include <stdio.h>
#include <string.h>
#include "bitmap.h"

#define PASS_DEBUG 0

inline uint32_t get_cell(GrayScale* image, int32_t x, int32_t y)
{
	if (x < 0 || (uint32_t)x >= image->width) return 0;
	if (y < 0 || (uint32_t)y >= image->height) return 0;
	return image->data[y * image->width + x];
}

inline void set_cell(GrayScale* image, int32_t x, int32_t y, uint8_t cell)
{
	if (x < 0 || (uint32_t)x >= image->width) return;
	if (y < 0 || (uint32_t)y >= image->height) return;
	image->data[y * image->width + x] = cell;
}

void erode_cells(GrayScale* output, GrayScale* input, GrayScale* final)
{
	uint32_t cells = 0;
	uint32_t i = 0;
	do 
	{
#if PASS_DEBUG == 1
		char buff[512];
		sprintf(buff, "res/pass%u.bmp", ++i);
		FILE* fp = fopen(buff, "wb");
		write_grayscale(fp, input);
		fclose(fp);	
#endif
		printf("Doing pass\n");
		erode_pass(output, input);
		{
			void* temp = output;
			output = input;
			input = temp;
		}
		cells = detect_pass(input, input, final);	
		printf("cells found = %u\n", cells);
	} 
	while (cells);
}

void erode_pass(GrayScale* output, GrayScale* input)
{
	for (int32_t y = 0; (uint32_t)y < input->width; y++)
	{
		for (int32_t x = 0; (uint32_t)x < input->height; x++)
		{
			set_cell(output, x, y, 
					get_cell(input, x, y)
					& get_cell(input, x - 1, y)
					& get_cell(input, x + 1, y)
					& get_cell(input, x, y - 1)
					& get_cell(input, x, y + 1)
			);
		}
	}
}

#define DETECT_SIZE 13

#define RSIZE 5

uint32_t detect_cell(GrayScale* output, GrayScale* input, GrayScale* final, int32_t cx, int32_t cy)
{
	uint32_t exclude = 0;
	for (int32_t x = cx - (DETECT_SIZE / 2 + 1); x < cx + (DETECT_SIZE / 2 + 1) + 1; x++)
	{
		exclude += get_cell(input, x, cy - (DETECT_SIZE / 2 + 1));
		exclude += get_cell(input, x, cy + (DETECT_SIZE / 2 + 1));
	}
	for (int32_t y = cy - (DETECT_SIZE / 2 + 1); y < cy + (DETECT_SIZE / 2 + 1) + 1; y++)
	{
		exclude += get_cell(input, cx - (DETECT_SIZE / 2 + 1), y);
		exclude += get_cell(input, cx + (DETECT_SIZE / 2 + 1), y);
	}
	if (exclude) return 1;

	uint32_t found = 0;
	for (int32_t y = cy - DETECT_SIZE / 2; y < cy + DETECT_SIZE / 2 + 1; y++)
	{
		for (int32_t x = cx - DETECT_SIZE / 2; x < cx + DETECT_SIZE / 2 + 1; x++)
		{
			found += get_cell(input, x, y);
		}	
	}
	if (!found) return 0;

	for (int32_t y = cy - DETECT_SIZE / 2; y < cy + DETECT_SIZE / 2 + 1; y++)
	{
		for (int32_t x = cx - DETECT_SIZE / 2; x < cx + DETECT_SIZE / 2 + 1; x++)
		{
			set_cell(output, x, y, 0);
		}
	}

	for (int32_t y = cy - RSIZE; y < cy + RSIZE + 1; y++)
	{
		for (int32_t x = cx - RSIZE; x < cx + RSIZE + 1; x++)
		{
	//		if ((cy - y) * (cy - y) + (cx - x) * (cx - x) > RSIZE * RSIZE) continue;

			set_cell(final, x, y, 255);
		}
	}

	return 2;
}

uint32_t detect_pass(GrayScale* output, GrayScale* input, GrayScale* final)
{
	memcpy(output->data, input->data, input->width * input->height);
	uint32_t cells = 0;
	for (uint32_t y = 0; y < input->height; y++)
	{
		for (uint32_t x = 0; x < input->width; x++)
		{
			cells |= detect_cell(output, input, final, x, y);
		}
	}
	return cells;
}
