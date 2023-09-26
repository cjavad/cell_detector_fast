#pragma once

#include <stdio.h>
#include <stdint.h>

typedef struct __attribute__((__packed__))
{
	uint8_t  magic[2];
	uint32_t size;
	uint16_t r0;
	uint16_t r1;
	uint32_t offset;
} BitmapHeader;

typedef struct __attribute__((__packed__))
{
	uint32_t size; // 40
	int32_t  width;
	int32_t  height;
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
} BitmapData;

typedef struct {
    BitmapHeader header;
    BitmapInfoHeader infoHeader;
    BitmapData bitmap;
} BitmapImage;

void read_bitmap(FILE* fp, BitmapImage* image);

void write_bitmap(FILE* fp, BitmapImage* image);

void create_bitmap(BitmapImage* bmp, uint32_t width, uint32_t height);

void print_bmpinfo(BitmapImage* image);
void free_bitmap(BitmapImage* image);


void draw_cross(BitmapData* bmp, uint32_t x, uint32_t y, uint8_t r, uint8_t g, uint8_t b);


uint32_t bmp_get_pixel_offset(BitmapData* bmp, uint32_t x, uint32_t y);
uint8_t bmp_get_pixel(BitmapData* bmp, uint32_t x, uint32_t y, uint8_t channel);
void bmp_set_offset(BitmapData *bmp, uint32_t offset, uint8_t r, uint8_t g, uint8_t b);
void bmp_set_offset_secure(BitmapData *bmp, uint32_t offset, uint8_t r, uint8_t g, uint8_t b);
void bmp_set_pixels(BitmapData *bmp, uint32_t x, uint32_t y, uint8_t r, uint8_t g, uint8_t b);