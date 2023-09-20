#include <stdint.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bitmap.h"
#include "grayscale.h"
#include "erode.h"
#include "samples.h"

#include "process.h"

#include "kernel.h"

int32_t main()
{
    uint32_t count;
    sample_t** samples;

    get_samples(&samples, &count, EASY);

    print_kernel();

    for (int i = 0; i < count; i++) {
        char* name = samples[i]->sample_name;

        printf("%s\n", name);
        FILE* fp = fopen(samples[i]->sample_path, "rb");
        BitmapImage inputImage;
        init_bitmap(&inputImage);
        read_bitmap(fp, &inputImage);
        fclose(fp);

        samples[i]->output_bmp = &inputImage;

        // mark_cells(&samples[i]->output_bmp->bitmap);
        kernel_pass(&samples[i]->output_bmp->bitmap);
        mark_cells(&samples[i]->output_bmp->bitmap);

        // grayscale_to_bmp(samples[i]->output_bmp, GrayScale *output);
        write_sample(samples[i]);

        break;
    }

    

	return 0;
}
