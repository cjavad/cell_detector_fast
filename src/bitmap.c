#include "bitmap.h"

#include <stdlib.h>
#include <stdio.h>

#include <string.h>

void read_bitmap(FILE* fp, BitmapImage* image)
{
	// read bitmap
	fseek(fp, 0, SEEK_END);
	uint32_t flen = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	void* file = malloc(flen);
	fread(file, 1, flen, fp);

    memcpy(&image->header, file, sizeof(BitmapHeader));
    memcpy(&image->infoHeader, file + sizeof(BitmapHeader), sizeof(BitmapInfoHeader));

	image->bitmap.width		= image->infoHeader.width;
	image->bitmap.height	= image->infoHeader.height;
	image->bitmap.byte_pp	= image->infoHeader.bpp >> 3;
	image->bitmap.row_width = (image->bitmap.width * image->bitmap.byte_pp + 3) & (~3);
	image->bitmap.data 		= malloc(image->bitmap.row_width * image->bitmap.height);

	// if (bitmap->row_width * bitmap->height != infoHeader->image_size); // oh no

	memcpy(image->bitmap.data, file + image->header.offset, image->bitmap.row_width * image->bitmap.height);

	free(file);
}

void write_bitmap(FILE *fp, BitmapImage *image) {
    // write bitmap
    uint32_t flen = image->header.offset + image->bitmap.row_width * image->bitmap.height;
    void* file = malloc(flen);

    memcpy(file, &image->header, sizeof(BitmapHeader));
    memcpy(file + sizeof(BitmapHeader), &image->infoHeader, sizeof(BitmapInfoHeader));
    memcpy(file + image->header.offset, image->bitmap.data, image->bitmap.row_width * image->bitmap.height);

    fwrite(file, 1, flen, fp);

    free(file);
}

void create_bitmap(BitmapImage* bmp, uint32_t width, uint32_t height)
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
    bmp->bitmap.data = malloc(bmp->bitmap.row_width * bmp->bitmap.height);
}

void print_bmpinfo(BitmapImage *image) {
    printf("Magic          = %.2s\n", (char *) &image->header.magic);
	printf("Size           = %ix%i\n", image->infoHeader.width, image->infoHeader.height);
	printf("Bits per pixel = %u\n", image->infoHeader.bpp);
}

void destroy_bitmap(BitmapImage* image) {
	free(image->bitmap.data);
}