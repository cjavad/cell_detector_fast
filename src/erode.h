#pragma once

#include "image.h"
#include "swizle.h"
#include <stdint.h>

void erode_pass(Image8u *output, Image8u* input, point_list_t* pixels);
void remove_pass(Image8u *output, Image8u* input, point_list_t* pixels);
void detect_pass(point_list_t* results, Image8u* image, point_list_t* pixels);