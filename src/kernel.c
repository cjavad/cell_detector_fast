#include "kernel.h"
#include "bitmap.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define ALIGN_8(x) (((x) + 7) & (~7))

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))
#define clamp(x, a, b) (max(min(x, b), a))
#define swap(a, b) {void* swap_temp = a; a = b; b = swap_temp;}

typedef struct {
	uint32_t width;
	uint32_t height;
	uint32_t offset;
	uint32_t stride;
	float* data;
} Image32f;

void write_image32f(Image32f* image, uint32_t id);
void write_kernel(Kernel* kernel, uint32_t id);

void image32f_from_bmp(Image32f* out, BitmapData* bmp)
{
	for (uint32_t y = 0; y < bmp->height; y++)
	{
		uint32_t bmp_offset = y * bmp->row_width;
		uint32_t img_offset = (y + out->offset) * out->stride + out->offset;
		for (uint32_t x = 0; x < bmp->width; x++) {
            out->data[img_offset + x] = (float) bmp->data[bmp_offset + x * 3] / 255.0f;
		}
	}
}

void kernel_instance(Image32f* out, Image32f* in, Kernel* kernel, int32_t cx, int32_t cy)
{
	float sum = 0;

    uint32_t base_x = cx + in->offset - kernel->size / 2;
    uint32_t base_y = cy + in->offset - kernel->size / 2;
	uint32_t img_base = base_y * in->stride + base_x;

	for (int32_t y = 0; y < kernel->size; y++)
	{
		uint32_t img_offset = img_base + y * in->stride;
		uint32_t ker_offset = y * kernel->size;

		for (int32_t x = 0; x < kernel->size; x++)
		{
			sum += kernel->data[ker_offset + x] * in->data[img_offset + x];
		}
	}

	out->data[(in->offset + cy) * in->stride + in->offset + cx] = sum;
}

void run_kernel(Image32f* out, Image32f* in, Kernel* kernel)
{
	for (int32_t x = 0; x < in->width; x++)
    {
        for (int32_t y = 0; y < in->height; y++)
        {
            kernel_instance(out, in, kernel, x, y);
        }
    }
}

void image32f_to_bitmap(Image32f* image, BitmapData* bmp) {
    for (uint32_t y = 0; y < image->height; y++)
	{
		uint32_t bmp_offset = y * bmp->row_width;
		uint32_t img_offset = (y + image->offset) * image->stride + image->offset;
		for (uint32_t x = 0; x < image->width; x++)
		{
            bmp_set_offset(
                bmp,
                bmp_offset + x * 3, 
                (uint8_t)(image->data[img_offset + x] * 255.0f),
                (uint8_t)(image->data[img_offset + x] * 255.0f),
                (uint8_t)(image->data[img_offset + x] * 255.0f)
            );
		}
	}
}

void kernel_pass(BitmapData* bmp, Kernel* kernel)
{
	Image32f image;

	image.width = bmp->width;
	image.height = bmp->height;
	image.offset = ALIGN_8(32);
	image.stride = image.width + 2 * image.offset;
	image.data = calloc(1, (image.width + 2 * image.offset) * (image.height + 2 * image.offset) * sizeof(float));

	Image32f buffer;

	buffer.width = image.width;
	buffer.height = image.height;
	buffer.offset = image.offset;
	buffer.stride = image.stride;
	buffer.data = calloc(1, (buffer.width + 2 * buffer.offset) * (buffer.height + 2 * buffer.offset) * sizeof(float));	

	//kfilter(&buffer, bmp, &pixels, 30);
	//write_image32f(&buffer, 0); 

    image32f_from_bmp(&buffer, bmp);
	run_kernel(&image, &buffer, kernel);
    free(buffer.data);    

	//normalize(&image, &image, &pixels);
	write_image32f(&image, 2);
    image32f_to_bitmap(&image, bmp);
    
    free(image.data);
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
            bmp_set_offset(
                &bmp.bitmap,
                bmp_offset + x * 3, 
                (uint8_t)(image->data[img_offset + x] * 255.0f),
                (uint8_t)(image->data[img_offset + x] * 255.0f),
                (uint8_t)(image->data[img_offset + x] * 255.0f)
            );
		}
	}

	write_bitmap(fp, &bmp);
    free_bitmap(&bmp);
	fclose(fp);
}

void write_kernel(Kernel* kernel, uint32_t id) {
    char buff[512];
	sprintf(buff, "res/kernel__%u.bmp", id);
	FILE* fp = fopen(buff, "wb");

	BitmapImage bmp;
	create_bitmap(&bmp, kernel->size, kernel->size);

	for (uint32_t y = 0; y < kernel->size; y++)
	{
		uint32_t bmp_offset = y * bmp.bitmap.row_width;
		for (uint32_t x = 0; x < kernel->size; x++)
		{
			bmp.bitmap.data[bmp_offset + x * 3 + 0] = (uint8_t)(kernel->data[y * kernel->size + x] * 255.0f * 10.0);
			bmp.bitmap.data[bmp_offset + x * 3 + 1] = (uint8_t)(kernel->data[y * kernel->size + x] * 255.0f * 10.0);
            bmp.bitmap.data[bmp_offset + x * 3 + 2] = (uint8_t)(kernel->data[y * kernel->size + x] * 255.0f * 10.0);
		}
	}

	write_bitmap(fp, &bmp);

	fclose(fp);
}

void init_kernel(Kernel* kernel, int32_t size, float sigma) 
{
    kernel->size = size;
    kernel->data = malloc(size * size * sizeof(float));

    int32_t half = size / 2;

    float sum = 0.0f;

    for (int32_t x = 0; x < size; x++) 
    {
        for (int32_t y = 0; y < size; y++) 
        {
            float dx = (float) (x - half);
            float dy = (float) (y - half);

            float d = 1.0 - sqrtf(dx * dx + dy * dy) / half;
            float weight = d * expf(-d * d / (2.0f * sigma * sigma));
            weight = d > 0.0 ? weight : 0.0f;

            kernel->data[y * size + x] = weight;
            sum += weight;
        }
    }

    for (int32_t i = 0; i < size * size; i++) 
    {
        kernel->data[i] /= sum;
    }

    printf("Kernel sum: %f\n", sum);
}

void free_kernel(Kernel *kernel) 
{
    free(kernel->data);
}
