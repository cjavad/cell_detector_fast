#include "bitmap.h"

#include <stdlib.h>
#include <stdio.h>

#include <string.h>

void read_bitmap(struct BitmapHeader* header, struct BitmapInfoHeader* infoHeader, struct Bitmap* bitmap, const char* filepath)
{
	// read bitmap
	FILE* fp = fopen(filepath, "r");
	fseek(fp, 0, SEEK_END);
	uint32_t flen = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	void* file = malloc(flen);
	fread(file, 1, flen, fp);

	fclose(fp);

	header->magic 					= *(uint16_t*)(file + 0 + 0);
	header->size 					= *(uint32_t*)(file + 0 + 2);
	header->r0 						= *(uint16_t*)(file + 0 + 6);
	header->r1 						= *(uint16_t*)(file + 0 + 8);
	header->offset 					= *(uint16_t*)(file + 0 + 10);

	infoHeader->size 				= *(uint32_t*)(file + 14 + 0);
	infoHeader->width 				= *( int32_t*)(file + 14 + 4);
	infoHeader->height				= *( int32_t*)(file + 14 + 8);
	infoHeader->planes				= *(uint16_t*)(file + 14 + 12);
	infoHeader->bpp					= *(uint16_t*)(file + 14 + 14);
	infoHeader->compression			= *(uint32_t*)(file + 14 + 16);
	infoHeader->image_size			= *(uint32_t*)(file + 14 + 20);
	infoHeader->h_resolution		= *(uint32_t*)(file + 14 + 24);
	infoHeader->v_resolution		= *(uint32_t*)(file + 14 + 28);
	infoHeader->palette_size		= *(uint32_t*)(file + 14 + 32);
	infoHeader->important_colors	= *(uint32_t*)(file + 14 + 36);

	bitmap->width		= infoHeader->width;
	bitmap->height		= infoHeader->height;
	bitmap->byte_pp		= infoHeader->bpp >> 3;
	bitmap->row_width 	= ((bitmap->width * bitmap->byte_pp + 3) >> 2) << 2;
	bitmap->data 		= malloc(bitmap->row_width * bitmap->height);

	// if (bitmap->row_width * bitmap->height != infoHeader->image_size); // oh no

	memcpy(bitmap->data, file + header->offset, bitmap->row_width * bitmap->height);

	free(file);
}