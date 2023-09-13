#pragma once

#include <stdint.h>

#include "grayscale.h"

void erode_cells(GrayScale* output, GrayScale* input, GrayScale* final);
void erode_pass(GrayScale* output, GrayScale* input);
uint32_t detect_pass(GrayScale* output, GrayScale* input, GrayScale* final);
