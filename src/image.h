#pragma once

#include "bitmap.h"
#include <stdint.h>

typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t offset;
	uint32_t stride;
    uint8_t* data;
} Image8u;

typedef struct {
	uint32_t width;
	uint32_t height;
	uint32_t offset;
	uint32_t stride;
	float* data;
} Image32f;

#define SWAP(a, b) {void* swap_temp = a; a = b; b = swap_temp;}
#define IMAGE_GET_OFFSET(image, x, y) (((image)->offset + (y)) * (image)->stride + (image)->offset + (x))

void init_image8u(Image8u *image, uint32_t width, uint32_t height, uint32_t offset);
void destroy_image8u(Image8u *image);

void init_image32f(Image32f *image, uint32_t width, uint32_t height, uint32_t offset);
void destroy_image32f(Image32f *image);

__attribute__((always_inline)) inline uint8_t image8u_get_pixel(Image8u *image, int32_t x, int32_t y) {
    return image->data[IMAGE_GET_OFFSET(image, x, y)];
}

__attribute__((always_inline)) inline void image8u_set_pixel(Image8u *image, int32_t x, int32_t y, uint8_t value) {
    image->data[IMAGE_GET_OFFSET(image, x, y)] = value;
}

__attribute__((always_inline)) inline float image32f_get_pixel(Image32f *image, int32_t x, int32_t y) {
    return image->data[IMAGE_GET_OFFSET(image, x, y)];
}

__attribute__((always_inline)) inline void image32f_set_pixel(Image32f *image, int32_t x, int32_t y, float value) {
    image->data[IMAGE_GET_OFFSET(image, x, y)] = value;
}

void image8u_from_image32f(Image8u *dest, Image32f *src);
void image32f_from_bmp(Image32f *dest, BitmapImage *src);
void image8u_to_bmp(BitmapImage *dest, Image8u *src);
void image32f_to_bmp(BitmapImage *dest, Image32f *src);
void write_image32f(FILE* fp, Image32f* image);
void write_image8u(FILE* fp, Image8u* image);


#define DEBUG_IMAGE32F(image, output) \
    fp = fopen(output, "wb"); \
    write_image32f(fp, image); \
    fclose(fp);


#define DEBUG_IMAGE8U(image, output) \
    fp = fopen(output, "wb"); \
    write_image8u(fp, image); \
    fclose(fp);