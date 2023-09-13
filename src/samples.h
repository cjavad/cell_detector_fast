#pragma once

#include <string.h>
#include <dirent.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "bitmap.h"


#define EASY 0
#define MEDIUM 1
#define HARD 2
#define IMPOSSIBLE 3

typedef struct {
    uint8_t sample_type;
    char* sample_name;
    char* sample_path;
    BitmapImage* output_bmp;
} sample_t;

// Figure this out.
void resolve_sample_path(char* output, uint8_t sample_type, char* sample_name);

void get_samples(sample_t*** samples, uint32_t* count, uint8_t sample_type);

void write_sample(sample_t* sample);
void write_samples(sample_t** samples, uint32_t count);