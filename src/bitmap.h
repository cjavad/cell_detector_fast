#pragma once

#include <stdint.h>

struct BitmapHeader
{
	uint16_t magic;
	uint32_t size;
	uint16_t r0;
	uint16_t r1;
	uint32_t offset;
};

struct BitmapInfoHeader
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
};

struct Bitmap 
{
	uint32_t width;
	uint32_t height;
	uint32_t row_width;
	uint32_t byte_pp;
	uint8_t* data;
};

void read_bitmap(struct BitmapHeader* header, struct BitmapInfoHeader* infoHeader, struct Bitmap* bitmap, const char* filepath);