#pragma once

#include <stdint.h>

#include "image.h"

void calc_grad(float* dx, float* dy, Image32f* image, int32_t x, int32_t y);

void gen_grad(Image32f* image, int id);