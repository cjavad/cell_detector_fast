#pragma once

#include "bitmap.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t length;
    uint64_t* data;
} Image1u; // 1 bit per pixel

typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t offset;
	uint32_t stride;
    uint8_t* data;
} Image8u; // 8 bits per pixel

typedef struct {
	uint32_t width;
	uint32_t height;
	uint32_t offset;
	uint32_t stride;
	float* data;
} Image32f; // 32 bits per pixel

#define SWAP(a, b) {void* swap_temp = a; a = b; b = swap_temp;}
#define IMAGE_GET_OFFSET(image, x, y) (((image)->offset + (y)) * (image)->stride + (image)->offset + (x))

void init_image1u(Image1u *image, uint32_t width, uint32_t height);
void destroy_image1u(Image1u *image);

void init_image8u(Image8u *image, uint32_t width, uint32_t height, uint32_t offset);
void destroy_image8u(Image8u *image);

void init_image32f(Image32f *image, uint32_t width, uint32_t height, uint32_t offset);
void destroy_image32f(Image32f *image);


__attribute__((always_inline)) inline uint8_t image1u_get_pixel(Image1u *image, int32_t x, int32_t y) {
    int c = ((y * image->width) + x) / 64;
    int o = ((y * image->width) + x) % 64;

    return (image->data[c] >> o) & 1;
}

__attribute__((always_inline)) inline void image1u_set_pixel(Image1u *image, int32_t x, int32_t y, bool value) {
    int c = ((y * image->width) + x) / 64;
    int o = ((y * image->width) + x) % 64;

    if (value) {
        image->data[c] |= (1 << o);
    } else {
        image->data[c] &= ~(1 << o);
    }
}


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

void image1u_from_image32f(Image1u *dest, Image32f *src, float threshold);
void image8u_from_image32f(Image8u *dest, Image32f *src);
void image32f_from_bmp(Image32f *dest, BitmapImage *src);
void image8u_to_bmp(BitmapImage *dest, Image8u *src);
void image32f_to_bmp(BitmapImage *dest, Image32f *src);

void write_image32f(FILE* fp, Image32f* image);
void write_image8u(FILE* fp, Image8u* image);
void write_image1u(FILE* fp, Image1u* image);

#define DEBUG_IMAGE32F(image, output) \
    fp = fopen(output, "wb"); \
    write_image32f(fp, image); \
    fclose(fp);


#define DEBUG_IMAGE8U(image, output) \
    fp = fopen(output, "wb"); \
    write_image8u(fp, image); \
    fclose(fp);

#define DEBUG_IMAGE1U(image, output) \
    fp = fopen(output, "wb"); \
    write_image1u(fp, image); \
    fclose(fp);


