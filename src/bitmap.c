#include "bitmap.h"

#include <stdlib.h>
#include <stdio.h>

#include <string.h>

void init_bitmap(BitmapImage* image) {
    image = malloc(sizeof(BitmapImage));
}

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
	image->bitmap.height		= image->infoHeader.height;
	image->bitmap.byte_pp		= image->infoHeader.bpp >> 3;
	image->bitmap.row_width 	= (image->bitmap.width * image->bitmap.byte_pp + 3) & (~3);
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

void print_bmpinfo(BitmapImage *image) {
    printf("Magic          = %.2s\n", (char *) &image->header.magic);
	printf("Size           = %ix%i\n", image->infoHeader.width, image->infoHeader.height);
	printf("Bits per pixel = %u\n", image->infoHeader.bpp);
}