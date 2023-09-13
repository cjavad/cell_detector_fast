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


void write_grayscale(FILE *fp, GrayScale *image) {
    BitmapHeader header;
    BitmapInfoHeader infoHeader;
    BitmapData data;
    BitmapImage bmpimage;

    bmpimage.bitmap = &data;
    bmpimage.header = &header;
    bmpimage.infoHeader = &infoHeader;

    uint32_t row_width = (3 * image->width + 3) & ~3;

    header.magic[0] = 0x42;
    header.magic[1] = 0x4D;
    header.size = sizeof(BitmapHeader) + sizeof(BitmapInfoHeader) + row_width * image->height;
    header.r0 = 0;
    header.r1 = 0;
    header.offset = sizeof(BitmapHeader) + sizeof(BitmapInfoHeader);

    infoHeader.size = sizeof(BitmapInfoHeader);
    infoHeader.width = image->width;
    infoHeader.height = image->height;
    infoHeader.planes = 1;
    infoHeader.bpp = 24;
    infoHeader.compression = 0;
    infoHeader.image_size = row_width * image->height;
    infoHeader.h_resolution = 0;
    infoHeader.v_resolution = 0;
    infoHeader.palette_size = 0;
    infoHeader.important_colors = 0;

    data.width = image->width;
    data.height = image->height;
    data.byte_pp = 3;
    data.row_width = row_width;
    data.data = malloc(data.row_width * data.height);

    // printf("row_width: %u\n", bitmap.row_width);
    // printf("height: %u\n", bitmap.height);
    // printf("width: %u\n", bitmap.width);
    // printf("pp: %u\n", bitmap.byte_pp);

    for (uint32_t y = 0; y < image->height; y++) {
        for (uint32_t x = 0; x < image->width; x++) {
            
            uint8_t b = image->data[y * image->width + x];
            uint8_t g = image->data[y * image->width + x];
            uint8_t r = image->data[y * image->width + x];

            data.data[y * data.row_width + x * data.byte_pp + 0] = r;
            data.data[y * data.row_width + x * data.byte_pp + 1] = g;
            data.data[y * data.row_width + x * data.byte_pp + 2] = b;
            

            //bitmap.data[y * bitmap.row_width + x * bitmap.byte_pp + 0] = y % 256;
            //bitmap.data[y * bitmap.row_width + x * bitmap.byte_pp + 1] = x % 256;
        }
    }

    write_bitmap(fp, &bmpimage);
}