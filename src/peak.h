#pragma once

#include <stdint.h>

#include "bitmap.h"
#include "vec.h"

typedef Vec(uint32_t) PeakVec;

void find_peaks(PeakVec* peaks, BitmapData* bmp);
