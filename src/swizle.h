#pragma once

#include <stdint.h>

#include "image.h"
#include "vec.h"

#include "bitmap.h"

typedef struct {int16_t x; int16_t y;} point_t;
typedef Vec(point_t) point_list_t;

void swizle_ma_jizle(point_list_t* whites, point_list_t* edges, BitmapData* bmp, uint32_t thold);
void swizle_ma_edges(Image8u* output, Image8u* input, point_list_t* edges, uint32_t thold);
void swizle_ma_whites(Image8u* output, Image8u* input, point_list_t* whites);