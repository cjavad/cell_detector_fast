#pragma once

#include <stdint.h>

#include "image.h"
#include "vec.h"
typedef struct {
	int16_t x;
	int16_t y;
	float dx;
	float dy;
} grad_point_t;

typedef Vec(grad_point_t) grad_point_list_t;


void calc_grad(float* dx, float* dy, Image32f* image, int32_t x, int32_t y);

void gen_grad(Image32f* image, point_list_t* out);

void grad_pass(BitmapData* input, int id);