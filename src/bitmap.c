#include "bitmap.h"

#include <stdlib.h>
#include <stdio.h>

#include <string.h>

void read_bitmap(FILE* fp, BitmapHeader* header, BitmapInfoHeader* infoHeader, Bitmap* bitmap)
{
	// read bitmap
	fseek(fp, 0, SEEK_END);
	uint32_t flen = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	void* file = malloc(flen);
	fread(file, 1, flen, fp);

    memcpy(header, file, sizeof(BitmapHeader));
    memcpy(infoHeader, file + sizeof(BitmapHeader), sizeof(BitmapInfoHeader));

	bitmap->width		= infoHeader->width;
	bitmap->height		= infoHeader->height;
	bitmap->byte_pp		= infoHeader->bpp >> 3;
	bitmap->row_width 	= ((bitmap->width * bitmap->byte_pp + 3) >> 2) << 2;
	bitmap->data 		= malloc(bitmap->row_width * bitmap->height);

	// if (bitmap->row_width * bitmap->height != infoHeader->image_size); // oh no

	memcpy(bitmap->data, file + header->offset, bitmap->row_width * bitmap->height);

	free(file);
}

void write_bitmap(FILE *fp, BitmapHeader *header, BitmapInfoHeader *infoHeader, Bitmap *bitmap) {
    // write bitmap
    uint32_t flen = header->offset + bitmap->row_width * bitmap->height;
    void* file = malloc(flen);

    memcpy(file, header, sizeof(BitmapHeader));
    memcpy(file + sizeof(BitmapHeader), infoHeader, sizeof(BitmapInfoHeader));
    memcpy(file + header->offset, bitmap->data, bitmap->row_width * bitmap->height);

    fwrite(file, 1, flen, fp);

    free(file);
}