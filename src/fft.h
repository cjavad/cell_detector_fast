#pragma once

#include <stdint.h>

#include "bitmap.h"

void fft_test(BitmapData* data);

void dft(float* real, float* imag, uint32_t width, uint32_t height);
void fft(float* real, float* imag, uint32_t width, uint32_t height);