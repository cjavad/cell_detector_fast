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

void process_samples(const int sample_type) {
    uint32_t count;
    sample_t** samples;

    get_samples(&samples, &count, sample_type);

    print_kernel();

    for (int i = 0; i < count; i++) {
        char* name = samples[i]->sample_name;

        printf("%s\n", name);
        FILE* fp = fopen(samples[i]->sample_path, "rb");
        BitmapImage inputImage;
        read_bitmap(fp, &inputImage);
        fclose(fp);
        
        // Initially output bmp is a copy of input bmp
        create_bitmap(samples[i]->output_bmp, inputImage.bitmap.width, inputImage.bitmap.height);

        samples[i]->output_bmp = &inputImage;

        fft_test(&inputImage.bitmap);

        // mark_cells(&&inputImage.bitmap);
        kernel_pass(&inputImage.bitmap);
        PeakVec peaks = find_peaks(&inputImage.bitmap);
        printf("peaks: %u\n", peaks.len);

        for (uint32_t j = 0; j < peaks.len; j++) {
            uint32_t peak = peaks.data[j];
            bmp_set_offset(&inputImage.bitmap, peak, 255, 0, 0);


            uint32_t x = (peak / inputImage.bitmap.byte_pp) % inputImage.bitmap.row_width;
            uint32_t y = (peak / inputImage.bitmap.byte_pp) / inputImage.bitmap.row_width;


            // Draw cross here with center at x, y
            draw_cross(&inputImage, x, y, 255, 0, 0);
    
        }

        // grayscale_to_bmp(samples[i]->output_bmp, GrayScale *output);
        write_sample(samples[i]);
    }
}

int32_t main()
{
    process_samples(EASY);
    process_samples(MEDIUM);
    process_samples(HARD);
    process_samples(IMPOSSIBLE);
	return 0;
}
