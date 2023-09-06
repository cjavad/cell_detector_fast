#pragma once

#include <stdio.h>
#include <stdint.h>

typedef struct __attribute__((__packed__))
{
	uint16_t magic;
	uint32_t size;
	uint16_t r0;
	uint16_t r1;
	uint32_t offset;
} BitmapHeader;

typedef struct __attribute__((__packed__))
{
	uint32_t size; // 40
	int32_t width;
	int32_t height;
	uint16_t planes;
	uint16_t bpp;
	uint32_t compression;
	uint32_t image_size;
	uint32_t h_resolution;
	uint32_t v_resolution;
	uint32_t palette_size;
	uint32_t important_colors;
} BitmapInfoHeader;

typedef struct
{
	uint32_t width;
	uint32_t height;
	uint32_t row_width;
	uint32_t byte_pp;
	uint8_t* data;
} Bitmap;

void read_bitmap(FILE* fp, BitmapHeader* header, BitmapInfoHeader* infoHeader, Bitmap* bitmap);

void write_bitmap(FILE* fp, BitmapHeader* header, BitmapInfoHeader* infoHeader, Bitmap* bitmap);