#pragma once

#include <stdint.h>

#include "grayscale.h"

typedef struct {
	uint32_t width;
	uint32_t height;
	float* data;
} Kernel;

typedef struct {
	uint32_t widht;
	uint32_t height;
	float* data;
} Image32f;

void kernel_init(Kernel* kernel, uint32_t width, uint32_t height);

void convolve(GrayScale* dst, GrayScale* src, Kernel* kernel);