#pragma once

#include <stdint.h>

#include "bitmap.h"
#include "image.h"
#include "vec.h"

typedef Vec(uint32_t) PeakVec;

void find_peaks(PeakVec* peaks, Image8u* bmp, uint8_t thold);
