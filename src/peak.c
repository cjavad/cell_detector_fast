#include "peak.h"

#include <stdint.h>

#include "bitmap.h"
#include "vec.h"

uint32_t find_highest_neighbor(BitmapData* bmp, uint32_t x, uint32_t y) 
{
    uint8_t best = 0;
    uint32_t bx = 0;
    uint32_t by = 0;

    for (int32_t dy = -1; dy <= 1; dy++)
    {
        for (int32_t dx = -1; dx <= 1; dx++) 
        {
            if (dx == 0 && dy == 0) continue;

            int32_t nx = x + dx;
            int32_t ny = y + dy;

            if (nx < 0 || ny < 0) continue;
            if (nx >= bmp->width || ny >= bmp->height) continue;

            uint32_t val = bmp_get_pixel(bmp, nx, ny, 0);
            if (val > best) 
            {
                best = val;
                bx = nx;
                by = ny;
            }
        }
    }

    return bmp_get_pixel_offset(bmp, bx, by);
}

uint32_t find_peak(BitmapData* bmp, uint32_t x, uint32_t y) {
    uint32_t index = bmp_get_pixel_offset(bmp, x, y);
    uint8_t val = bmp->data[index];

    while (1) 
    {   
        uint32_t neighbor = find_highest_neighbor(bmp, x, y);
        uint8_t nval = bmp->data[neighbor]; 

        if (nval <= val) return index;
        index = neighbor;
        x = neighbor % bmp->row_width / bmp->byte_pp;
        y = neighbor / bmp->row_width;
        val = nval;
    }
} 

void remove_peak(BitmapData* bmp, uint32_t x, uint32_t y) 
{
    uint32_t index = bmp_get_pixel_offset(bmp, x, y);
    uint8_t val = bmp->data[index];
    bmp->data[index + 0] = 0;

    for (int32_t dy = -1; dy <= 1; dy++) 
    {
        for (int32_t dx = -1; dx <= 1; dx++) 
        {
            if (dx == 0 && dy == 0) continue;

            int32_t nx = x + dx;
            int32_t ny = y + dy;

            if (nx < 0 || ny < 0) continue;
            if (nx >= bmp->width || ny >= bmp->height) continue;

            uint8_t neighbor = bmp_get_pixel(bmp, nx, ny, 0);

            if (neighbor == 0) continue;
            if (neighbor <= val) remove_peak(bmp, nx, ny);
        }
    }
}

void find_peaks(PeakVec* peaks, BitmapData* bmp) {
    for (uint32_t y = 0; y < bmp->height; y++) 
    {
        for (uint32_t x = 0; x < bmp->width; x++)
        {
            if (bmp_get_pixel(bmp, x, y, 0) == 0) continue;

            uint32_t peak = find_peak(bmp, x, y);
            uint32_t px = peak % bmp->row_width / bmp->byte_pp;
            uint32_t py = peak / bmp->row_width;
            remove_peak(bmp, px, py);

            vec_push(peaks, peak);
        }
    }
}
