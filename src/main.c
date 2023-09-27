#include <stdint.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "bitmap.h"
#include "erode.h"
#include "grayscale.h"
#include "image.h"
#include "peak.h"
#include "samples.h"

#include "process.h"

#include "kernel.h"

#include "fft.h"
#include "swizle.h"
#include "vec.h"

uint32_t mode = 0;

Vec(Kernel) kernels;
uint32_t kernel_type = 0;
uint32_t kernel_size = 0;
float kernel_arg = 0;
float kernel_arg2 = 0;
uint32_t sample_type = EASY;
char* pass_dir = NULL;
char* input = NULL;
char* output = NULL;

void process_bitmap(BitmapImage *image) {
    point_list_t whites;
    point_list_t edges;

    vec_init(&whites);
    vec_init(&edges);

    swizle_ma_jizle(&whites, &edges, &image->bitmap, 88);

    Image32f in, out;
    Image32f* in_ptr = &in;
    Image32f* out_ptr = &out;

    init_image32f(&in, image->bitmap.width, image->bitmap.height, 32);
    init_image32f(&out, image->bitmap.width, image->bitmap.height, 32);
    image32f_from_bmp(&in, image);

    for (uint32_t i = 0; i < kernels.len; i++) {
        kernel_pass(out_ptr, in_ptr, &kernels.data[i]);
        SWAP(in_ptr, out_ptr)

        if (pass_dir != NULL) {
            FILE* fp;
            char buff[512];
            sprintf(buff, "%s/kernel_pass_%u.bmp", pass_dir, i);
            DEBUG_IMAGE32F(in_ptr, buff);
        }
    } 

    destroy_image32f(out_ptr);

    Image8u grayscale, buffer;

    Image8u* grayscale_ptr = &grayscale;
    Image8u* buffer_ptr = &buffer;

    init_image8u(&grayscale, image->bitmap.width, image->bitmap.height, 32);
    init_image8u(&buffer, image->bitmap.width, image->bitmap.height, 32);

    image8u_from_image32f(&buffer, in_ptr);
    destroy_image32f(in_ptr);


    swizle_ma_edges(&grayscale, &buffer, &edges, 60);
    memset(buffer.data, 0, buffer.stride * (buffer.height + 2 * buffer.offset));

    swizle_ma_whites(&buffer, &grayscale, &whites);
    memset(grayscale.data, 0, grayscale.stride * (grayscale.height + 2 * grayscale.offset));

    for (uint32_t i = 0; i < 15; i++) {
        erode_pass(grayscale_ptr, buffer_ptr, &whites);
        remove_pass(grayscale_ptr, buffer_ptr, &whites);
        SWAP(grayscale_ptr, buffer_ptr)

        FILE* fp;
        char buff[512];
        sprintf(buff, "%s/erode_pass_%u.bmp", pass_dir, i);
        DEBUG_IMAGE8U(buffer_ptr, buff);
    }
    
    image8u_to_bmp(image, &buffer);



    /*

    PeakVec peaks;
    vec_init(&peaks);

    find_peaks(&peaks, &img);

    printf("Found %u peaks\n", peaks.len);

    // bmp_filter(&image->bitmap, 88);

    for (uint32_t j = 0; j < peaks.len; j++) {
        uint32_t peak = peaks.data[j];
        img.data[peak] = 255;

        uint32_t x = peak % img.stride - img.offset;
        uint32_t y = peak / img.stride - img.offset;

        // Draw cross here with center at x, y on top of image
        draw_cross(&image->bitmap, x, y, 255, 0, 0);
    }

    if (pass_dir != NULL) {
        FILE* fp;
        char buff[512];
        sprintf(buff, "%s/peaks.bmp", pass_dir);
        DEBUG_IMAGE8U(&img, buff);

        sprintf(buff, "%s/debug.bmp", pass_dir);
        DEBUG_BMP(&bmp, buff);

    }
    vec_free(&peaks);

    //*/

    destroy_image8u(&grayscale);
    destroy_image8u(&buffer);
}

void process_samples() {
    uint32_t sample_count;
    sample_t** samples;

    get_samples(&samples, &sample_count, sample_type); 

    for (uint32_t i = 0; i < sample_count; i++) {
        char* name = samples[i]->sample_name;
        printf("Processing sample %s\n", name);
;
        FILE* fp = fopen(samples[i]->sample_path, "rb");
        BitmapImage inputImage;
        read_bitmap(fp, &inputImage);
        fclose(fp);

        samples[i]->output_bmp = &inputImage; 

        process_bitmap(&inputImage);

        printf("Heyo!\n");

        write_sample(samples[i]);
        printf("Heyo 1.5!\n");
        free_sample(samples[i]);

        printf("Heyo 2!\n");
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
#define OPT_KERNEL_ARG2 6
#define OPT_INPUT 7
#define OPT_OUTPUT 8
#define OPT_PASS_DIR 9

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
        case 3:
            printf("Initializing LoG with size: %d sigma: %f and scale %f\n", kernel_size, kernel_arg, kernel_arg2);
            init_log_kernel(&kernel, kernel_size, kernel_arg, kernel_arg2);
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
    
    for (uint32_t i = 0; i < argc; i++) {
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

        if (mode == OPT_KERNEL_ARG2) {
            kernel_arg2 = atof(argv[i]);
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

        if (strcmp(argv[i], "-b") == 0 || strcmp(argv[i], "--kernel-arg2") == 0) {
            mode = OPT_KERNEL_ARG2;
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
        printf("  3\t LoG\n");

        return 0;
    }

    printf("Running with %d kernels\n", kernels.len);

    // if not pass_dir exists, create it
    struct stat st;
    
    if (!S_ISDIR(stat(pass_dir, &st) == 0 ? st.st_mode : 0)) {
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
    for (uint32_t i = 0; i < kernels.len; i++) {
        free_kernel(&kernels.data[i]);
    }

    vec_free(&kernels);

	return 0;
}
