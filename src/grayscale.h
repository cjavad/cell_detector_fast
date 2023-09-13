#pragma once

#include <stdint.h>
#include "bitmap.h"

typedef struct {
    uint32_t width;
    uint32_t height;
    uint8_t* data;
} GrayScale;


void bmp_to_grayscale(Bitmap* bitmap, GrayScale* image);

void init_gray(GrayScale* dest, uint32_t widht, uint32_t height);

// For Debug
void write_grayscale(FILE *fp, GrayScale* image);

