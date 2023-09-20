#include "kernel.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define ALIGN_8(x) (((x) + 7) & (~7))

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))
#define clamp(x, a, b) (max(min(x, b), a))
#define swap(a, b) {void* swap_temp = a; a = b; b = swap_temp;}


#include "kernel.i"

typedef struct {
	uint32_t count;
	struct {
		uint16_t x;
		uint16_t y;
	}
	p[952 * 952 / 4];
} PixelList;

typedef struct {
	uint32_t width;
	uint32_t height;
	float* data;
} Kernel;

typedef struct {
	uint32_t width;
	uint32_t height;
	uint32_t offset;
	uint32_t stride;
	float* data;
} Image32f;

void write_image32f(Image32f* image, uint32_t id);

void kfilter(Image32f* out, BitmapData* bmp, PixelList* pixels, uint8_t thold)
{
	for (uint32_t y = 0; y < bmp->height; y++)
	{
		uint32_t bmp_offset = y * bmp->row_width;
		uint32_t img_offset = (y + out->offset) * out->stride + out->offset;
		for (uint32_t x = 0; x < bmp->width; x++)
		{
			if (bmp->data[bmp_offset + x * 3] > thold) {
				out->data[img_offset + x] = 1.0f;
				pixels->p[pixels->count].x = x;
				pixels->p[pixels->count].y = y;
				pixels->count++;
			} else {
				out->data[img_offset + x] = 0.0f;
			}
		}
	}
}

void normalize(Image32f* out, Image32f* in, PixelList* pixels)
{
	float max = 0;
	float min = 100;
	for (uint32_t i = 0; i < pixels->count; i++)
	{
		uint32_t x = pixels->p[i].x;
		uint32_t y = pixels->p[i].y;

		if (in->data[(y + in->offset) * in->stride + in->offset + x] > max)
		{
			max = in->data[(y + in->offset) * in->stride + in->offset + x];
		}
		if (in->data[(y + in->offset) * in->stride + in->offset + x] < min)
		{
			min = in->data[(y + in->offset) * in->stride + in->offset + x];
		}
	}

	max = 1 / (max - min);

	for (uint32_t y = 0; y < in->height; y++)
	{
		for (uint32_t x = 0; x < in->width; x++)
		{
			out->data[(y + out->offset) * out->stride + out->offset + x] = 
				in->data[(y + in->offset) * in->stride + in->offset + x] * max - min;
		}
	}
}

void kernel_instance(Image32f* out, Image32f* in, int32_t cx, int32_t cy)
{
	float sum = 0;

    uint32_t base_x = cx + in->offset - KERNEL_HALF;
    uint32_t base_y = cy + in->offset - KERNEL_HALF;
	uint32_t img_base = base_y * in->stride + base_x;

	for (int32_t y = 0; y < KERNEL_SIZE; y++)
	{
		uint32_t img_offset = img_base + y * in->stride;
		uint32_t ker_offset = y * KERNEL_SIZE;

		for (int32_t x = 0; x < KERNEL_SIZE; x++)
		{
			sum += kernel[ker_offset + x] * in->data[img_offset + x];
		}
	}

	out->data[(in->offset + cy) * in->stride + in->offset + cx] = sum;
}

void run_kernel(Image32f* out, Image32f* in, PixelList* pixels)
{
	for (uint32_t i = 0; i < pixels->count; i++) 
	{
		kernel_instance(out, in, pixels->p[i].x, pixels->p[i].y);
	}
}

void kernel_pass(BitmapData* bmp)
{
	Image32f image;
	image.width = bmp->width;
	image.height = bmp->height;
	image.offset = ALIGN_8(KERNEL_SIZE - KERNEL_HALF);
	image.stride = image.width + 2 * image.offset;
	image.data = calloc(1, (image.width + 2 * image.offset) * (image.height + 2 * image.offset) * sizeof(float));

	Image32f buffer;
	buffer.width = image.width;
	buffer.height = image.height;
	buffer.offset = image.offset;
	buffer.stride = image.stride;
	buffer.data = calloc(1, (buffer.width + 2 * buffer.offset) * (buffer.height + 2 * buffer.offset) * sizeof(float));
	// printf("width = %u\nheight=%u\noffset=%u\nstride=%u\n", image.width, image.height, image.offset, image.stride);
	// if (!image.data) printf("fuck\n");
	PixelList pixels;
	pixels.count = 0;

	kfilter(&buffer, bmp, &pixels, 88);
	write_image32f(&buffer, 0);

	run_kernel(&image, &buffer, &pixels);

	//normalize(&image, &image, &pixels);
	write_image32f(&image, 2);
}


void write_image32f(Image32f* image, uint32_t id)
{
	char buff[512];
	sprintf(buff, "res/kernel%u.bmp", id);
	FILE* fp = fopen(buff, "wb");

	BitmapImage bmp;
	create_bitmap(&bmp, image->height, image->width);

	for (uint32_t y = 0; y < image->height; y++)
	{
		uint32_t bmp_offset = y * bmp.bitmap.row_width;
		uint32_t img_offset = (y + image->offset) * image->stride + image->offset;
		for (uint32_t x = 0; x < image->width; x++)
		{
			bmp.bitmap.data[bmp_offset + x * 3 + 0] = (uint8_t)(image->data[img_offset + x] * 255.0f);
			bmp.bitmap.data[bmp_offset + x * 3 + 1] = (uint8_t)(image->data[img_offset + x] * 255.0f);
			bmp.bitmap.data[bmp_offset + x * 3 + 2] = (uint8_t)(image->data[img_offset + x] * 255.0f);
		}
	}

	write_bitmap(fp, &bmp);

	fclose(fp);
}

float kernel_weight(int32_t x, int32_t y)
{
    float dist = sqrt((x * x) + (y * y)); 
    return dist > KERNEL_HALF ? 0 : dist; 
}

void print_kernel()
{
    float norm = 0;

    for (int32_t y = -KERNEL_HALF; y <= KERNEL_HALF; y++)
    {
        for (int32_t x = -KERNEL_HALF; x <= KERNEL_HALF; x++)
        {
            norm += kernel_weight(x, y);
        }
    }

	FILE* fp = fopen("../../src/kernel.i", "w");
	fprintf(fp, "float kernel[] = {\n");
	for (int32_t y = -KERNEL_HALF; y <= KERNEL_HALF; y++)
	{
		fprintf(fp, "\t");
		for (int32_t x = -KERNEL_HALF; x <= KERNEL_HALF; x++)
		{
			float weight = kernel_weight(x, y) / fabs(norm);
			fprintf(fp, "%ff, ", weight);
		}
		fprintf(fp, "\n");
	}

	fprintf(fp, "};");
	fclose(fp);
}
