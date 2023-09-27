#include "erode.h"
#include "image.h"
#include "swizle.h"
#include "vec.h"
#include <stdint.h>
#include <stdio.h>


void erode_pass(Image8u *output, Image8u *input, point_list_t *pixels) {
    uint32_t width = output->width;
    for (uint32_t i = 0; i < pixels->len; i++) {
        int32_t x = pixels->data[i].x;
        int32_t y = pixels->data[i].y;

        image8u_set_pixel(output, x, y, 
            image8u_get_pixel(input, x, y) 
            & image8u_get_pixel(input, x - 1, y)
            & image8u_get_pixel(input, x + 1, y)
            & image8u_get_pixel(input, x, y - 1)
            & image8u_get_pixel(input, x, y + 1)
        );
    }
}

void remove_pass(Image8u *output, Image8u *input, point_list_t *pixels) {
    uint32_t write = 0;

	for (uint32_t read = 0; read < pixels->len; read++)
	{
		int32_t x = pixels->data[read].x;
		int32_t y = pixels->data[read].y;
        uint32_t offset = IMAGE_GET_OFFSET(output, x, y);
		
        if (output->data[offset] == 0) {
			input->data[offset] = 0;
			continue;
		}

		pixels->data[write].x = x;
		pixels->data[write].y = y;
		write++; 
	}

	pixels->len = write;
}

#define DETECT_SIZE 11

void detect_pass(point_list_t* results, Image8u* image, point_list_t* pixels) {
    for (uint32_t i = 0; i < pixels->len; i++)
	{
		int32_t cx = pixels->data[i].x;
		int32_t cy = pixels->data[i].y;


        // exclusion zone check
        {
            int32_t minx = cx - (DETECT_SIZE / 2 + 1);
            int32_t maxx = cx + (DETECT_SIZE / 2 + 1) + 1;

            int32_t miny = cy - (DETECT_SIZE / 2 + 1);
            int32_t maxy = cy + (DETECT_SIZE / 2 + 1) + 1;

            uint32_t exclude = 0;
            for (int32_t x = minx; x < maxx; x++)
            {
                exclude += image->data[IMAGE_GET_OFFSET(image, x, miny)];
                exclude += image->data[IMAGE_GET_OFFSET(image, x, maxy)];
            }

            if (exclude) return;

            for (int32_t y = miny; y < maxy; y++)
            {
                exclude += image->data[IMAGE_GET_OFFSET(image, minx, y)];
                exclude += image->data[IMAGE_GET_OFFSET(image, maxx, y)];
            }

            if (exclude) return;
        }

        printf("detecting %d %d which has value %d\n", cx, cy, image8u_get_pixel(image, cx, cy));

        // cell check
        {
            int32_t minx = cx - (DETECT_SIZE / 2);
            int32_t maxx = cx + (DETECT_SIZE / 2) + 1;

            int32_t miny = cy - (DETECT_SIZE / 2);
            int32_t maxy = cy + (DETECT_SIZE / 2) + 1;

            uint32_t found = 0;

            for (int32_t y = miny; y < maxy; y++)
            {
                for (int32_t x = minx; x < maxx; x++)
                {
                    found += image->data[IMAGE_GET_OFFSET(image, x, y)];
                }	
            }

            if (!found) return;

            // remove cell
            for (int32_t y = miny; y < maxy; y++)
            {
                for (int32_t x = minx; x < maxx; x++)
                {
                    image->data[IMAGE_GET_OFFSET(image, x, y)] = 0;
                }
            }

            vec_push(results, ((point_t){cx, cy}));
        }

	}
}