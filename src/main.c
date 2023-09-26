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

int mode = 0;

Vec(Kernel) kernels;
int kernel_type = 0;
int kernel_size = 0;
float kernel_arg = 0;
int sample_type = EASY;
char* pass_dir = NULL;
char* input = NULL;
char* output = NULL;

void process_bitmap(BitmapImage *image) {
    BitmapImage bmp;
    clone_bitmap(&bmp, image);

    for (uint32_t i = 0; i < kernels.len; i++) {
        kernel_pass(&bmp.bitmap, &kernels.data[i]);

        if (pass_dir != NULL) {
            FILE* fp;
            char buff[512];
            sprintf(buff, "%s/kernel_pass_%u.bmp", pass_dir, i);
            DEBUG_BMP(&bmp, buff);
        }
    }

    PeakVec peaks;
    vec_init(&peaks);

    find_peaks(&peaks, &bmp.bitmap);

    printf("Found %u peaks\n", peaks.len);

    for (uint32_t j = 0; j < peaks.len; j++) {
        uint32_t peak = peaks.data[j];
        bmp_set_offset(&bmp.bitmap, peak, 255, 0, 0);

        uint32_t x = (peak % bmp.bitmap.row_width) / bmp.bitmap.byte_pp;
        uint32_t y = peak / bmp.bitmap.row_width;

        // Draw cross here with center at x, y on top of image
        draw_cross(&image->bitmap, x, y, 255, 0, 0);
    }

    free_bitmap(&bmp);
    vec_free(&peaks);
}

void process_samples() {
    uint32_t sample_count;
    sample_t** samples;

    get_samples(&samples, &sample_count, sample_type); 

    for (int i = 0; i < sample_count; i++) {
        char* name = samples[i]->sample_name;
        printf("Processing sample %s\n", name);
;
        FILE* fp = fopen(samples[i]->sample_path, "rb");
        BitmapImage inputImage;
        read_bitmap(fp, &inputImage);
        fclose(fp);

        samples[i]->output_bmp = &inputImage; 

        process_bitmap(&inputImage);
        write_sample(samples[i]);
        free_sample(samples[i]);
    }

    free(samples);
}

void process_single() {
    FILE* fp = fopen(input, "rb");
    BitmapImage inputImage;
    read_bitmap(fp, &inputImage);
    fclose(fp);

    process_bitmap(&inputImage);

    fp = fopen(output, "wb");
    write_bitmap(fp, &inputImage);
    fclose(fp);
}

#define OPT_DEFAULT 0
#define OPT_HELP 1
#define OPT_SAMPLE_TYPE 2
#define OPT_KERNEL 3
#define OPT_KERNEL_SIZE 4
#define OPT_KERNEL_ARG 5
#define OPT_INPUT 6 
#define OPT_OUTPUT 7
#define OPT_PASS_DIR 8

void create_kernel() {
    // Initialize kernel
    Kernel kernel;
    
    switch (kernel_type) {
        case 1:
            printf("Initializing gaussian kernel with size: %d sigma: %f\n", kernel_size, kernel_arg);
            init_gaussian_kernel(&kernel, kernel_size, kernel_arg);
            break;
        case 2:
            printf("Initializing laplacian kernel with size: %d\n", kernel_size);
            init_laplacian_kernel(&kernel, kernel_size);
            break;
        default:
            break;
    }

    if (pass_dir != NULL) {
        FILE* fp;
        char buff[512];
        sprintf(buff, "%s/kernel_%u.bmp", pass_dir, kernels.len);
        fp = fopen(buff, "wb");
        write_kernel(fp, &kernel);
        fclose(fp);
    }

    vec_push(&kernels, kernel);
}

int32_t main(int argc, char** argv)
{
    vec_init(&kernels);
    
    for (int i = 0; i < argc; i++) {
        if (mode == OPT_SAMPLE_TYPE) {
            sample_type = atoi(argv[i]);
            mode = OPT_DEFAULT;
            continue;
        }

        if (mode == OPT_KERNEL) {
            if (kernel_type != 0) create_kernel();
            kernel_type = atoi(argv[i]);
            mode = OPT_DEFAULT;
            continue;
        }

        if (mode == OPT_KERNEL_SIZE) {
            kernel_size = atoi(argv[i]);
            mode = OPT_DEFAULT;
            continue;
        }

        if (mode == OPT_KERNEL_ARG) {
            kernel_arg = atof(argv[i]);
            mode = OPT_DEFAULT;
            continue;
        }

        if (mode == OPT_INPUT) {
            input = argv[i];
            mode = OPT_DEFAULT;
            continue;
        }

        if (mode == OPT_OUTPUT) {
            output = argv[i];
            mode = OPT_DEFAULT;
            continue;
        }

        if (mode == OPT_PASS_DIR) {
            pass_dir = argv[i];
            mode = OPT_DEFAULT;
            continue;
        }

        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            mode = OPT_HELP;
            break;
        }

        if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--sample-type") == 0) {
            mode = OPT_SAMPLE_TYPE;
            continue;
        }

        if (strcmp(argv[i], "-k") == 0 || strcmp(argv[i], "--kernel") == 0) {
            mode = OPT_KERNEL;
            continue;
        }

        if (strcmp(argv[i], "-z") == 0 || strcmp(argv[i], "--kernel-size") == 0) {
            mode = OPT_KERNEL_SIZE;
            continue;
        }

        if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--kernel-arg") == 0) {
            mode = OPT_KERNEL_ARG;
            continue;
        }

        if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--input") == 0) {
            mode = OPT_INPUT;
            continue;
        }

        if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) {
            mode = OPT_OUTPUT;
            continue;
        }

        if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--pass-dir") == 0) {
            mode = OPT_PASS_DIR;
            continue;
        }
    }

    if (kernel_type != 0) create_kernel();

    printf("Running with %d kernels\n", kernels.len);

    if (mode == OPT_HELP) {
        printf("Usage: %s [options]\n", argv[0]);
        printf("Options:\n");
        printf("  -h --help\t Show this help message\n");
        printf("  -s --sample-type\t Set sample type\n");
        printf("  -k --kernel\t Set kernel type\n");
        printf("  -z --kernel-size\t Set kernel size\n");
        printf("  -a --kernel-arg\t Set kernel argument\n");
        printf("  -i --input\t Set input file\n");
        printf("  -o --output\t Set output file\n");
        printf("  -p --pass-dir\t Set pass directory\n");

        // Print sample types
        printf("Sample types:\n");
        printf("  0\t EASY\n");
        printf("  1\t MEDIUM\n");
        printf("  2\t HARD\n");
        printf("  3\t EXTREME\n");

        // Print kernel types
        printf("Kernel types:\n");
        printf("  0\t NONE\n");
        printf("  1\t GAUSSIAN\n");
        printf("  2\t LAPLACIAN\n");

        return 0;
    }

    // if not pass_dir exists, create it
    if (pass_dir != NULL) {
        mkdir(pass_dir, 0777);
    }

    if (input == NULL ^ output == NULL) {
        printf("Input and output both need to be set.\n");
    } else if (input != NULL && output != NULL) {
        // Apply on single image
        process_single();
    } else {
        // Process samples
        process_samples();
    }

    // Free kernels
    for (int i = 0; i < kernels.len; i++) {
        free_kernel(&kernels.data[i]);
    }

    vec_free(&kernels);

	return 0;
}
