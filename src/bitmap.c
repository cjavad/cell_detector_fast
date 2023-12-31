#include "bitmap.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <string.h>

void read_bitmap(FILE* fp, BitmapImage* image)
{
	// Read bitmap
	fseek(fp, 0, SEEK_END);
	uint32_t flen = ftell(fp);
	fseek(fp, 0, SEEK_SET);

// We know all the files have the same constant size
// so this is also a constant allocation
	void* file = malloc(flen);
	fread(file, 1, flen, fp);

    memcpy(&image->header, file, sizeof(BitmapHeader));
    memcpy(&image->infoHeader, file + sizeof(BitmapHeader), sizeof(BitmapInfoHeader));

	image->bitmap.width		= image->infoHeader.width;
	image->bitmap.height	= image->infoHeader.height;
	image->bitmap.byte_pp	= image->infoHeader.bpp >> 3;
	image->bitmap.row_width = (image->bitmap.width * image->bitmap.byte_pp + 3) & (~3);

// All our images have a constant size, so this can be a constant allocation
	image->bitmap.data 		= malloc(image->bitmap.row_width * image->bitmap.height);

	// if (bitmap->row_width * bitmap->height != infoHeader->image_size); // oh no

	memcpy(image->bitmap.data, file + image->header.offset, image->bitmap.row_width * image->bitmap.height);

	free(file);
}

void write_bitmap(FILE *fp, BitmapImage *image) {
    // write bitmap
    uint32_t flen = image->header.offset + image->bitmap.row_width * image->bitmap.height;

// This is also a constant allocation
    void* file = malloc(flen);

    memcpy(file, &image->header, sizeof(BitmapHeader));
    memcpy(file + sizeof(BitmapHeader), &image->infoHeader, sizeof(BitmapInfoHeader));
    memcpy(file + image->header.offset, image->bitmap.data, image->bitmap.row_width * image->bitmap.height);

    fwrite(file, 1, flen, fp);

    free(file);
}

void init_bitmap(BitmapImage* bmp, uint32_t width, uint32_t height)
{
	uint32_t row_width = (3 * width + 3) & ~3;

    bmp->header.magic[0] = 0x42;
    bmp->header.magic[1] = 0x4D;
    bmp->header.size = sizeof(BitmapHeader) + sizeof(BitmapInfoHeader) + row_width * height;
    bmp->header.r0 = 0;
    bmp->header.r1 = 0;
    bmp->header.offset = sizeof(BitmapHeader) + sizeof(BitmapInfoHeader);

    bmp->infoHeader.size = sizeof(BitmapInfoHeader);
    bmp->infoHeader.width = width;
    bmp->infoHeader.height = height;
    bmp->infoHeader.planes = 1;
    bmp->infoHeader.bpp = 24;
    bmp->infoHeader.compression = 0;
    bmp->infoHeader.image_size = row_width * height;
    bmp->infoHeader.h_resolution = 0;
    bmp->infoHeader.v_resolution = 0;
    bmp->infoHeader.palette_size = 0;
    bmp->infoHeader.important_colors = 0;

    bmp->bitmap.width = width;
    bmp->bitmap.height = height;
    bmp->bitmap.byte_pp = 3;
    bmp->bitmap.row_width = row_width;

// This is also a constant allocation
    bmp->bitmap.data = calloc(1, bmp->bitmap.row_width * bmp->bitmap.height);
}

void bmp_filter(BitmapData *bmp, uint32_t threshold) {
    for (uint32_t y = 0; y < bmp->height; y++) {
        for (uint32_t x = 0; x < bmp->width; x++) {
            uint32_t offset = bmp_get_pixel_offset(bmp, x, y);
            uint8_t r = bmp->data[offset + 0];
            uint8_t g = bmp->data[offset + 1];
            uint8_t b = bmp->data[offset + 2];

            if (r < threshold || g < threshold || b < threshold) {
                bmp_set_offset(bmp, offset, 0, 0, 0);
            } else {
                bmp_set_offset(bmp, offset, 255, 255, 255);
            }
        }
    }
}

void free_bitmap(BitmapImage *image) {
    free(image->bitmap.data);
}

void clone_bitmap(BitmapImage *dst, BitmapImage *src) {
    memcpy(dst, src, sizeof(BitmapImage));

// This is also a constant allocation
    dst->bitmap.data = malloc(dst->bitmap.row_width * dst->bitmap.height);
    memcpy(dst->bitmap.data, src->bitmap.data, dst->bitmap.row_width * dst->bitmap.height);
}

void print_bmpinfo(BitmapImage *image) {
    printf("Magic          = %.2s\n", (char *) &image->header.magic);
	printf("Size           = %ix%i\n", image->infoHeader.width, image->infoHeader.height);
	printf("Bits per pixel = %u\n", image->infoHeader.bpp);
}


void destroy_bitmap(BitmapImage* image) {
	free(image->bitmap.data);
}


#define CROSS_SIZE 9
#define CROSS_THICKNESS 2

uint32_t draw_cross(BitmapData *bmp, uint32_t x, uint32_t y, uint8_t r, uint8_t g, uint8_t b, uint8_t thold) {
    if (
        bmp_get_pixel_secure(bmp, x + 0, y, 0) < thold ||
        bmp_get_pixel_secure(bmp, x + 1, y, 0) < thold ||
        bmp_get_pixel_secure(bmp, x - 1, y, 0) < thold ||
        bmp_get_pixel_secure(bmp, x, y + 1, 0) < thold ||
        bmp_get_pixel_secure(bmp, x, y - 1, 0) < thold
    
    ) return 0;

    // printf("Cell at (%i, %i)\n", x, y);

    for (uint32_t i = 0; i < CROSS_SIZE; i++) {
        for (uint32_t j = 0; j < CROSS_THICKNESS; j++) {
            bmp_set_pixels(bmp, x + i, y + j, r, g, b);
            bmp_set_pixels(bmp, x + i, y - j, r, g, b);
            bmp_set_pixels(bmp, x - i, y + j, r, g, b);
            bmp_set_pixels(bmp, x - i, y - j, r, g, b);
            bmp_set_pixels(bmp, x + j, y + i, r, g, b);
            bmp_set_pixels(bmp, x - j, y + i, r, g, b);
            bmp_set_pixels(bmp, x + j, y - i, r, g, b);
            bmp_set_pixels(bmp, x - j, y - i, r, g, b);
        }
    }

    bmp_set_pixels(bmp, x, y, 255, 0, 0);

    return 1;
}