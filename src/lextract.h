#pragma once

#include <stdint.h>

#include "image.h"
#include "vec.h"

#include "bitmap.h"

void lextract_points(point_list_t* whites, point_list_t* edges, BitmapData* bmp, uint32_t thold);
void lextract_edges(Image8u* output, Image8u* input, point_list_t* edges, uint32_t thold);
void lextract_whites(Image8u* output, Image8u* input, point_list_t* whites);