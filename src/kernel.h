#pragma once

#include <stdint.h>

#include "bitmap.h"

#define KERNEL_SIZE (int32_t) 25
#define KERNEL_HALF (int32_t) (KERNEL_SIZE / 2)

void kernel_pass(BitmapData* bmp);

void print_kernel();
