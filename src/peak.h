#pragma once

#include <stdint.h>

#include "bitmap.h"
#include "vec.h"

typedef Vec(uint32_t) PeakVec;

PeakVec find_peaks(BitmapData* bmp);
