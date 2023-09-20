#include "grayscale.h"
#include "bitmap.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void bmp_to_grayscale(BitmapData *bitmap, GrayScale *image) {
    image->width = bitmap->width;
    image->height = bitmap->height;
    image->data = malloc(image->width * image->height);

    for (uint32_t y = 0; y < image->height; y++) {
        for (uint32_t x = 0; x < image->width; x++) {
            uint8_t r = bitmap->data[y * bitmap->row_width + x * bitmap->byte_pp + 0];
            uint8_t g = bitmap->data[y * bitmap->row_width + x * bitmap->byte_pp + 1];
            uint8_t b = bitmap->data[y * bitmap->row_width + x * bitmap->byte_pp + 2];

            // Filter really close-to-black pixels away from the r channel e(only one we care about)
            image->data[y * image->width + x] = r ;//>= 128 ? 255 : 0;
        }
    }
}

void init_gray(GrayScale* dest, uint32_t width, uint32_t height)
{
	dest->width = width;
	dest->height = height;
	dest->data = malloc(width * height * sizeof(uint8_t));
}

void grayscale_to_bmpdata(BitmapData *dest, GrayScale *src) {
    // printf("Writing grayscale\n");

    for (uint32_t y = 0; y < src->height; y++) {
        for (uint32_t x = 0; x < src->width; x++) {
            uint8_t b = src->data[y * src->width + x];
            uint8_t g = src->data[y * src->width + x];
            uint8_t r = src->data[y * src->width + x];

            dest->data[y * dest->row_width + x * dest->byte_pp + 0] = r;
            dest->data[y * dest->row_width + x * dest->byte_pp + 1] = g;
            dest->data[y * dest->row_width + x * dest->byte_pp + 2] = b;
        }
    }
}

void grayscale_to_bmp(BitmapImage *bmp, GrayScale *image) {
    grayscale_to_bmpdata(&bmp->bitmap, image);
}


void write_grayscale(FILE *fp, GrayScale *image) {
    BitmapImage bmp;
    create_bitmap(&bmp, image->width, image->height);
    grayscale_to_bmp(&bmp, image);
    write_bitmap(fp, &bmp);
}