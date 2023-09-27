#include "kernel.h"
#include "bitmap.h"
#include "image.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))
#define clamp(x, a, b) (max(min(x, b), a))

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

	out->data[(in->offset + cy) * in->stride + in->offset + cx] = clamp(sum, 0.0, 1.0);
}

void kernel_pass(Image32f* out, Image32f* in, Kernel* kernel)
{
	for (int32_t x = 0; x < in->width; x++)
    {
        for (int32_t y = 0; y < in->height; y++)
        {
            kernel_instance(out, in, kernel, x, y);
        }
    }
}

void write_kernel(FILE* fp, Kernel* kernel) {
	BitmapImage bmp;
	init_bitmap(&bmp, kernel->size, kernel->size);

	for (uint32_t y = 0; y < kernel->size; y++)
	{
		uint32_t bmp_offset = y * bmp.bitmap.row_width;
		for (uint32_t x = 0; x < kernel->size; x++)
		{
			bmp.bitmap.data[bmp_offset + x * 3 + 0] = (uint8_t)(kernel->data[y * kernel->size + x] * 255.0f * 100.0);
			bmp.bitmap.data[bmp_offset + x * 3 + 1] = (uint8_t)(kernel->data[y * kernel->size + x] * 255.0f * 100.0);
            bmp.bitmap.data[bmp_offset + x * 3 + 2] = (uint8_t)(kernel->data[y * kernel->size + x] * 255.0f * 100.0);
		}
	}

	write_bitmap(fp, &bmp);
    free_bitmap(&bmp);
}

void print_kernel(Kernel *kernel) {
    // Print in text as matrix
    for (int32_t y = 0; y < kernel->size; y++) {
        for (int32_t x = 0; x < kernel->size; x++) {
            printf("%f ", kernel->data[y * kernel->size + x]);
        }
        printf("\n");
    }
}

#define KERNEL_INIT(kernel, size) \
    kernel->size = size; \
    kernel->data = malloc(size * size * sizeof(float)); \
    int32_t half = size / 2;

#define KERNEL_NORM_START \
    float sum = 0.0f;

#define KERNEL_NORM_INC(kernel, size, x, y) \
    sum += kernel->data[y * size + x];

#define KERNEL_NORM_END(kernel, size) \
    for (int32_t i = 0; i < size * size; i++) { \
        kernel->data[i] /= fabs(sum); \
    }


void init_gaussian_kernel(Kernel* kernel, int32_t size, float scale) {
    KERNEL_INIT(kernel, size)
    KERNEL_NORM_START

    for (int32_t x = 0; x < size; x++) {
        for (int32_t y = 0; y < size; y++) {
            float dx = (float) (x - half);
            float dy = (float) (y - half);

            float d = dx * dx + dy * dy;
            float t = 2.0f * scale * scale;

            kernel->data[y * size + x] = expf(-d / t) / (2.0f * M_PI * t);
            KERNEL_NORM_INC(kernel, size, x, y)
        }
    }

    KERNEL_NORM_END(kernel, size)
}


void init_laplacian_kernel(Kernel *kernel, int32_t size) {
    KERNEL_INIT(kernel, size)
    KERNEL_NORM_START

    for (int32_t x = 0; x < size; x++) {
        for (int32_t y = 0; y < size; y++) {
            float dx = (float) (x - half);
            float dy = (float) (y - half);

            if (dx == 0 && dy == 0) {
                kernel->data[y * size + x] = -(size - 1) * (size - 1);
            } else {
                kernel->data[y * size + x] = half - (fabs(dx) + fabs(dy) - 1); 
                if (kernel->data[y * size + x] < 0) kernel->data[y * size + x] = 0;
            }
            KERNEL_NORM_INC(kernel, size, x, y)
        }
    }

    KERNEL_NORM_END(kernel, size)
}

void init_blur_kernel(Kernel* kernel, int32_t size, float sigma) 
{
    KERNEL_INIT(kernel, size)
    KERNEL_NORM_START

    for (int32_t x = 0; x < size; x++) 
    {
        for (int32_t y = 0; y < size; y++) 
        {
            float dx = (float) (x - half);
            float dy = (float) (y - half);

            float d = sqrtf(dx * dx + dy * dy);
            float weight = 1.0 - d / half;
            weight = d < half ? weight : 0.0f;

            kernel->data[y * size + x] = weight;
            KERNEL_NORM_INC(kernel, size, x, y)
        }
    }

    for (int32_t i = 0; i < size * size; i++) 
    {
        kernel->data[i] /= fabs(sum);
        kernel->data[i] *= 0.9;
    }
}


// https://homepages.inf.ed.ac.uk/rbf/HIPR2/log.htm Laplacian of Gaussian
void init_log_kernel(Kernel *kernel, int32_t size, float sigma, float scale)
{
    KERNEL_INIT(kernel, size)    

    for (int32_t x = 0; x < size; x++) 
    {
        for (int32_t y = 0; y < size; y++) 
        {
            float dx = (float) (x - half);
            float dy = (float) (y - half);

            // x**2 + y**2
            float d = dx * dx + dy * dy;

            // 2 * sigma**2
            float t = 2.0f * sigma * sigma;

            kernel->data[y * size + x] = scale * (-1.0f / (M_PI * sigma * sigma * sigma * sigma)) * (1.0f - (d / t)) * expf(-d / t);
        }
    }
}

void free_kernel(Kernel *kernel) 
{
    free(kernel->data);
}
