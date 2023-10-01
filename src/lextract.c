#include "lextract.h"
#include "image.h"
#include "vec.h"
#include <stdint.h>

/**
*  log extract points for edges (and whites)
*/
void lextract_points(point_list_t* whites, point_list_t* edges, BitmapData* bmp, uint32_t thold)
{
	for (uint32_t y = 0; y < bmp->height; y++)
	{
		for (uint32_t x = 0; x < bmp->width; x++)
		{
			if (bmp_get_pixel(bmp, x, y, 0) <= thold) continue;

			vec_push(whites, ((point_t){x, y}));

			if (
				bmp_get_pixel_secure(bmp, x - 1, y, 0) > thold &&
				bmp_get_pixel_secure(bmp, x + 1, y, 0) > thold &&
				bmp_get_pixel_secure(bmp, x, y - 1, 0) > thold &&
				bmp_get_pixel_secure(bmp, x, y + 1, 0) > thold
			) continue;

			vec_push(edges, ((point_t){x, y}));
		}
	}
}

/**
* Flood fill for every edge point, only copy found pixels to output.
*/
void lextract_edges(Image8u *output, Image8u *input, point_list_t *edges, uint32_t thold) {    
    point_list_t stack;
    vec_init(&stack);
    
    for (uint32_t i = 0; i < edges->len; i++) {
        stack.len = 0;
        vec_push(&stack, edges->data[i]);

        while (stack.len > 0) {
            point_t p = vec_pop(&stack);

            if (image8u_get_pixel(input, p.x, p.y) <= thold) continue;

            image8u_set_pixel(output, p.x, p.y, 255);
            image8u_set_pixel(input,  p.x, p.y, 0);

            vec_push(&stack, ((point_t){p.x - 1, p.y}));
            vec_push(&stack, ((point_t){p.x + 1, p.y}));
            vec_push(&stack, ((point_t){p.x, p.y - 1}));
            vec_push(&stack, ((point_t){p.x, p.y + 1}));
        }
    }


    vec_free(&stack);
}

/**
* Invert based on whites.
*/
void lextract_whites(Image8u* output, Image8u* input, point_list_t* whites) {
    for (uint32_t i = 0; i < whites->len; i++) {
        point_t p = whites->data[i];
        image8u_set_pixel(output, p.x, p.y, 255 - image8u_get_pixel(input, p.x, p.y));
    }
}