#ifndef BMP_H
#define BMP_H

#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int width;
    int height;
    int channels;
    int row_size;
    int pixel_array_size;
    int file_size;
} BMP;

typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} RGB;

BMP *bmp_read_header(FILE* fp);
RGB *bmp_read_pixels(FILE* fp, BMP *bmp);
void bmp_free(BMP *bmp);
void bmp_write(FILE* fp, BMP *bmp);

#endif