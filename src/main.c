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
        BitmapImage inputImage;
        init_bitmap(&inputImage);
        read_bitmap(fp, &inputImage);
        fclose(fp);

        samples[i]->output_bmp = &inputImage;

        // grayscale_to_bmp(samples[i]->output_bmp, GrayScale *output);
        write_sample(samples[i]);

    }

	return 0;
}
