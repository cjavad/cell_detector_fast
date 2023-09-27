#include "swizle.h"
#include <stdint.h>

/**
*  Swizle ma jizle for them edges (and whites)
*/
void swizle_ma_jizle(point_list_t* whites, point_list_t* edges, BitmapData* bmp, uint32_t thold)
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