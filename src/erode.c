#include "erode.h"
#include "image.h"
#include <stdint.h>


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