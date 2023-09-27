#pragma once

#include <stdint.h>

#include "bitmap.h"
#include "image.h"

typedef struct {
    int32_t size;
    float* data;
} Kernel;


void write_kernel(FILE* fp, Kernel* kernel);
void print_kernel(Kernel* kernel);

void free_kernel(Kernel* kernel);

void kernel_pass(Image32f* out, Image32f* in, Kernel* kernel);

void init_blur_kernel(Kernel* kernel, int32_t size, float sigma);
void init_gaussian_kernel(Kernel* kernel, int32_t size, float scale_t);
void init_laplacian_kernel(Kernel* kernel, int32_t size);
void init_log_kernel(Kernel* kernel, int32_t size, float sigma, float scale);