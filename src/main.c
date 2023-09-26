#include <stdint.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "bitmap.h"
#include "grayscale.h"
#include "peak.h"
#include "samples.h"

#include "process.h"

#include "kernel.h"

#include "fft.h"
#include "vec.h"

void process_samples(const int sample_type) {
    uint32_t count;
    sample_t** samples;

    Kernel first_kernel;
    init_kernel(&first_kernel, 9, 0.0f);

    write_kernel(&first_kernel, 0);

    get_samples(&samples, &count, sample_type); 

    for (int i = 0; i < count; i++) {
        char* name = samples[i]->sample_name;

        printf("%s\n", name);
        FILE* fp = fopen(samples[i]->sample_path, "rb");
        BitmapImage inputImage;
        read_bitmap(fp, &inputImage);
        fclose(fp);
        
        // Initially output bmp is a copy of input bmp
        // create_bitmap(samples[i]->output_bmp, inputImage.bitmap.width, inputImage.bitmap.height);

        samples[i]->output_bmp = &inputImage; 

        // mark_cells(&&inputImage.bitmap);
        PeakVec peaks;
        vec_init(&peaks);
        kernel_pass(&inputImage.bitmap, &first_kernel);
        find_peaks(&peaks, &inputImage.bitmap);
        printf("peaks: %u\n", peaks.len);

        for (uint32_t j = 0; j < peaks.len; j++) {
            uint32_t peak = peaks.data[j];
            bmp_set_offset(&inputImage.bitmap, peak, 255, 0, 0);


            uint32_t x = (peak % inputImage.bitmap.row_width) / inputImage.bitmap.byte_pp;
            uint32_t y = peak / inputImage.bitmap.row_width;


            // Draw cross here with center at x, y
            draw_cross(&inputImage.bitmap, x, y, 255, 0, 0);
    
        }
        
        vec_free(&peaks);

        // grayscale_to_bmp(samples[i]->output_bmp, GrayScale *output);
        write_sample(samples[i]);
        free_sample(samples[i]);
    }

    free_kernel(&first_kernel);
    free(samples);
}

int32_t main()
{
    process_samples(EASY);
    process_samples(MEDIUM);
    process_samples(HARD);
    process_samples(IMPOSSIBLE);
	return 0;
}
