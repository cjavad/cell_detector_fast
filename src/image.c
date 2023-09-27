#include "image.h"
#include "bitmap.h"
#include <stdlib.h>

#define ALIGN(x, a) (((x) + ((a) - 1)) & (~((a) - 1)))
#define ALIGN_8(x) ALIGN(x, 8)
#define ALIGN_32(x) ALIGN(x, 32)

void init_image1u(Image1u *image, uint32_t width, uint32_t height) {
    image->width = width;
    image->height = height;
    
    // Data contains 64 bit integers (8 bytes) for 64 pixels
    // 8 * width / 64 = width / 8
    image->length = ALIGN_8(height * width) / 8;
    image->data = calloc(1, image->length * sizeof(uint64_t));
}

void destroy_image1u(Image1u *image) {
    free(image->data);
}

void init_image8u(Image8u *image, uint32_t width, uint32_t height, uint32_t offset) {
    image->width = width;
    image->height = height;
    image->offset = ALIGN_32(offset);
    image->stride = width + 2 * image->offset;
    image->data = calloc(1, image->stride * (image->height + 2 * image->offset));
}

void destroy_image8u(Image8u *image) {
    free(image->data);
}

void init_image32f(Image32f *image, uint32_t width, uint32_t height, uint32_t offset) {
    image->width = width;
    image->height = height;
    image->offset = ALIGN_32(offset);
    image->stride = width + 2 * image->offset;
    image->data = calloc(1, image->stride * (image->height + 2 * image->offset) * sizeof(float));
}

void destroy_image32f(Image32f *image) {
    free(image->data);
}


void image1u_from_image32f(Image1u *dest, Image32f *src, float threshold) {
    for (uint32_t y = 0; y < src->height; y++)
    {
        for (uint32_t x = 0; x < src->width; x++)
        {
            image1u_set_pixel(dest, x, y, image32f_get_pixel(src, x, y) > threshold);
        }
    }
}

void image1u_to_bmp(BitmapImage *bmp, Image1u *image) {
    for (uint32_t y = 0; y < image->height; y++)
    {

        for (uint32_t x = 0; x < image->width; x++)
        {
            bmp_set_pixels(
                &bmp->bitmap,
                x,
                y,
                image1u_get_pixel(image, x, y) * 255,
                image1u_get_pixel(image, x, y) * 255,
                image1u_get_pixel(image, x, y) * 255
            );
        }
    }
}

void image32f_to_bitmap(Image32f* image, BitmapData* bmp) {
    for (uint32_t y = 0; y < image->height; y++)
	{
		uint32_t bmp_offset = y * bmp->row_width;
		uint32_t img_offset = (y + image->offset) * image->stride + image->offset;
		for (uint32_t x = 0; x < image->width; x++)
		{
            bmp_set_offset(
                bmp,
                bmp_offset + x * 3, 
                (uint8_t)(image->data[img_offset + x] * 255.0f),
                (uint8_t)(image->data[img_offset + x] * 255.0f),
                (uint8_t)(image->data[img_offset + x] * 255.0f)
            );
		}
	}
}


void image32f_from_bmp(Image32f *dest, BitmapImage *src) {
    for (uint32_t y = 0; y < src->bitmap.height; y++)
	{
		uint32_t bmp_offset = y * src->bitmap.row_width;
		uint32_t img_offset = (y + dest->offset) * dest->stride + dest->offset;

		for (uint32_t x = 0; x < src->bitmap.width; x++) {
            dest->data[img_offset + x] = (float) src->bitmap.data[bmp_offset + x * 3] / 255.0f;
		}
	}
}


void image32f_from_image8u(Image32f *dest, Image8u *src) {
    for (uint32_t y = 0; y < src->height; y++)
    {
        uint32_t img_offset = (y + dest->offset) * dest->stride + dest->offset;
        uint32_t src_offset = (y + src->offset) * src->stride + src->offset;

        for (uint32_t x = 0; x < src->width; x++) {
            dest->data[img_offset + x] = (float) src->data[src_offset + x] / 255.0f;
        }
    }
}


void image8u_from_image32f(Image8u *dest, Image32f *src) {
    for (uint32_t y = 0; y < src->height; y++)
    {
        uint32_t img_offset = (y + dest->offset) * dest->stride + dest->offset;
        uint32_t src_offset = (y + src->offset) * src->stride + src->offset;

        for (uint32_t x = 0; x < src->width; x++) {
            dest->data[img_offset + x] = (uint8_t) (src->data[src_offset + x] * 255.0f);
        }
    }
}


void image32f_to_bmp(BitmapImage *bmp, Image32f *image) {
    for (uint32_t y = 0; y < image->height; y++)
	{
		uint32_t bmp_offset = y * bmp->bitmap.row_width;
		uint32_t img_offset = (y + image->offset) * image->stride + image->offset;

		for (uint32_t x = 0; x < image->width; x++)
		{
            bmp_set_offset(
                &bmp->bitmap,
                bmp_offset + x * 3, 
                (uint8_t)(image->data[img_offset + x] * 255.0f),
                (uint8_t)(image->data[img_offset + x] * 255.0f),
                (uint8_t)(image->data[img_offset + x] * 255.0f)
            );
		}
	}
}


void write_image32f(FILE* fp, Image32f* image)
{
	BitmapImage bmp;
	init_bitmap(&bmp, image->height, image->width);
    image32f_to_bmp(&bmp, image);
	write_bitmap(fp, &bmp);
    free_bitmap(&bmp);
}

void image8u_to_bmp(BitmapImage *bmp, Image8u* image) {
      for (uint32_t y = 0; y < image->height; y++)
    {
        uint32_t bmp_offset = y * bmp->bitmap.row_width;
        uint32_t img_offset = (y + image->offset) * image->stride + image->offset;
        for (uint32_t x = 0; x < image->width; x++)
        {
            bmp_set_offset(
                &bmp->bitmap,
                bmp_offset + x * 3, 
                image->data[img_offset + x],
                image->data[img_offset + x],
                image->data[img_offset + x]
            );
        }
    }
}

void write_image8u(FILE* fp, Image8u *image) {
    BitmapImage bmp;
    init_bitmap(&bmp, image->height, image->width);
    image8u_to_bmp(&bmp, image);
    write_bitmap(fp, &bmp);
    free_bitmap(&bmp);
}

void write_image1u(FILE* fp, Image1u *image) {
    BitmapImage bmp;
    init_bitmap(&bmp, image->height, image->width);
    image1u_to_bmp(&bmp, image);
    write_bitmap(fp, &bmp);
    free_bitmap(&bmp);
}