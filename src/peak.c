#include "peak.h"

#include <stdint.h>
#include <stdio.h>

#include "bitmap.h"
#include "image.h"
#include "vec.h"

uint32_t find_highest_neighbor(Image8u* img, uint32_t x, uint32_t y) 
{
    uint8_t best = 0;
    uint32_t bx = 0;
    uint32_t by = 0;

    #define RANGE 2

    for (int32_t dy = -RANGE; dy <= RANGE; dy++)
    {
        for (int32_t dx = -RANGE; dx <= RANGE; dx++) 
        {
            if (dx == 0 && dy == 0) continue;

            int32_t nx = x + dx;
            int32_t ny = y + dy;

            if (nx < 0 || ny < 0) continue;
            if (nx >= img->width || ny >= img->height) continue;

            uint32_t val = image8u_get_pixel(img, nx, ny);

            if (val > best) 
            {
                best = val;
                bx = nx;
                by = ny;
            }
        }
    }

    return IMAGE_GET_OFFSET(img, bx, by);
}

uint32_t find_peak(Image8u* img, uint32_t x, uint32_t y) {
    uint32_t index = IMAGE_GET_OFFSET(img, x, y);
    uint8_t val = img->data[index];

    while (1) 
    {   
        uint32_t neighbor = find_highest_neighbor(img, x, y);
        uint8_t nval = img->data[neighbor]; 

        if (nval <= val) return index;
        
        index = neighbor;
        x = neighbor % img->stride - img->offset;
        y = neighbor / img->stride - img->offset;
        val = nval;
    }
}

#define MAX_DIST 15

void remove_peak(Image8u* img, uint32_t x, uint32_t y) 
{
    PeakVec indices;
    vec_init(&indices);

    vec_push(&indices, IMAGE_GET_OFFSET(img, x, y));

    int32_t cx = x;
    int32_t cy = y;

    while (indices.len > 0) {
        uint32_t index = vec_pop(&indices);

        x = index % img->stride - img->offset;
        y = index / img->stride - img->offset;

        uint8_t val = img->data[index];
        img->data[index] = 0;

        for (int32_t dy = -1; dy <= 1; dy++) 
        {
            for (int32_t dx = -1; dx <= 1; dx++) 
            {
                if (dx == 0 && dy == 0) continue;

                int32_t nx = x + dx;
                int32_t ny = y + dy;

                if (nx < 0 || ny < 0) continue;
                if (nx >= img->width || ny >= img->height) continue;

                if (abs(nx - cx) > MAX_DIST || abs(ny - cy) > MAX_DIST) continue;
 
                uint8_t neighbor = image8u_get_pixel(img, nx, ny);

                if (neighbor == 0 || neighbor > val) continue;

                uint32_t nindex = IMAGE_GET_OFFSET(img, nx, ny);
                vec_push(&indices, nindex);
            }
        }
    }

    vec_free(&indices);
}

void find_peaks(PeakVec* peaks, Image8u* image, uint8_t thold) {
    for (uint32_t y = 0; y < image->height; y++) 
    {
        for (uint32_t x = 0; x < image->width; x++)
        {
            if (image8u_get_pixel(image, x, y) < thold) continue;

            uint32_t peak = find_peak(image, x, y);

            uint32_t px = peak % image->stride - image->offset;
            uint32_t py = peak / image->stride - image->offset;

            remove_peak(image, px, py);

            vec_push(peaks, peak);
        }
    }
}
