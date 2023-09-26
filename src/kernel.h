#pragma once

#include <stdint.h>

#include "bitmap.h"

typedef struct {
    int32_t size;
    float* data;
} Kernel;

void write_kernel(Kernel* kernel, uint32_t id);

void init_kernel(Kernel* kernel, int32_t size, float sigma);

void free_kernel(Kernel* kernel);

void kernel_pass(BitmapData* bmp, Kernel* kernel);
