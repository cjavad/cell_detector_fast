#pragma once

#include <stdint.h>
#include "bitmap.h"

typedef struct {
    uint32_t width;
    uint32_t height;
    uint8_t* data;
} GrayScale;

void bmp_to_grayscale(GrayScale* image, BitmapData* bitmap);

void init_gray(GrayScale* dest, uint32_t widht, uint32_t height);

void grayscale_to_bmpdata(BitmapData* dest, GrayScale* src);
void grayscale_to_bmp(BitmapImage *bmp, GrayScale *image);

// For Debug
void write_grayscale(FILE *fp, GrayScale* image);

#define DEBUG_GRAYSCALE(grayscale, output) \
    fp = fopen(output, "wb"); \
    write_grayscale(fp, grayscale); \
    fclose(fp);
