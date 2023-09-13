#include <stdint.h>

#include <stdio.h>
#include <stdlib.h>

#include "bitmap.h"
#include "grayscale.h"
#include "erode.h"
#include "samples.h"

int32_t main()
{
    uint32_t count;
    sample_t** samples;

    get_samples(&samples, &count, EASY);


    for (int i = 0; i < count; i++) {
        printf("%s\n", samples[i]->sample_path);

        FILE* fp = fopen(samples[i]->sample_path, "rb");

        // Do logic here.

        samples[i]->output_bmp;

        fclose(fp);
    }

	return 0;
}
