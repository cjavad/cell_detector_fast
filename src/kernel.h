#pragma once

#include <stdint.h>

#include "bitmap.h"

#define KERNEL_SIZE 32
#define KERNEL_OFFSET 16

void kernel_pass(BitmapData* bmp);

void print_kernel();