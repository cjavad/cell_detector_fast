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

#include "grad.h"


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
#define OPT_METHOD 10

uint32_t mode = 0;

Vec(Kernel) kernels;

#define KERNEL_TYPE_NONE 0
#define KERNEL_TYPE_GAUSSIAN 1
#define KERNEL_TYPE_LAPLACIAN 2
#define KERNEL_TYPE_LOG 3
#define KERNEL_TYPE_DOG 4

uint32_t kernel_type = 0;
uint32_t kernel_size = 0;
float kernel_arg = 0;
float kernel_arg2 = 0;

#define METHOD_NONE 0
#define METHOD_ERODE 1
#define METHOD_PEEKPOINTS 2
#define METHOD_GRADE 3

uint32_t method = 0;

// See sample types in samples.h
uint32_t sample_type = EASY;
char* pass_dir = NULL;
char* input = NULL;
char* output = NULL;


// Global variables for debugging
FILE* fp;
char buff[512];

void erode(point_list_t* cells, BitmapImage* image, Image8u* grayscale_ptr, Image8u* buffer_ptr) {
    point_list_t results;
    point_list_t whites;
    point_list_t edges;

    vec_init(&results);
    vec_init(&whites);
    vec_init(&edges);

    swizle_ma_jizle(&whites, &edges, &image->bitmap, 88);

    swizle_ma_edges(grayscale_ptr, buffer_ptr, &edges, 60);
    memset(buffer_ptr->data, 0, buffer_ptr->stride * (buffer_ptr->height + 2 * buffer_ptr->offset));

    swizle_ma_whites(buffer_ptr, grayscale_ptr, &whites);
    memset(grayscale_ptr->data, 0, grayscale_ptr->stride * (grayscale_ptr->height + 2 * grayscale_ptr->offset));

    for (uint32_t i = 0; whites.len > 0; i++) {
        printf("Pixel list len: %d\n", whites.len);
        erode_pass(grayscale_ptr, buffer_ptr, &whites);
        remove_pass(grayscale_ptr, buffer_ptr, &whites);


        detect_pass(&results, grayscale_ptr, &whites);
        remove_pass(grayscale_ptr, buffer_ptr, &whites);
        
        SWAP(grayscale_ptr, buffer_ptr)

        if (pass_dir != NULL) {
            sprintf(buff, "%s/erode_pass_%u.bmp", pass_dir, i);
            DEBUG_IMAGE8U(buffer_ptr, buff);
        }
    }
    
    // image8u_to_bmp(image, &buffer);
    printf("Found %u results\n", results.len);

    for (uint32_t i = 0; i < results.len; i++) {
        uint32_t x = results.data[i].x;
        uint32_t y = results.data[i].y;

        vec_push(cells, ((point_t){x, y}));
    }

    vec_free(&edges);
    vec_free(&whites);
    vec_free(&results);
}

void peekpoints(point_list_t* cells, Image8u* buffer_ptr) {
    PeakVec peaks;
    vec_init(&peaks);

    find_peaks(&peaks, buffer_ptr);

    printf("Found %u peaks\n", peaks.len);

    for (uint32_t j = 0; j < peaks.len; j++) {
        uint32_t peak = peaks.data[j];
        buffer_ptr->data[peak] = 255;

        uint32_t x = peak % buffer_ptr->stride - buffer_ptr->offset;
        uint32_t y = peak / buffer_ptr->stride - buffer_ptr->offset;

        // Draw cross here with center at x, y on top of image
        vec_push(cells, ((point_t){x, y}));
    }

    if (pass_dir != NULL) {
        FILE* fp;
        char buff[512];
        sprintf(buff, "%s/peaks.bmp", pass_dir);
        DEBUG_IMAGE8U(buffer_ptr, buff);
    }

    vec_free(&peaks);
}


void grade(point_list_t* cells, Image32f* image) {
    gen_grad(image, cells);
}

void process_bitmap(BitmapImage *image) {
    point_list_t cells;
    vec_init(&cells);

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

    init_image8u(buffer_ptr, image->bitmap.width, image->bitmap.height, 32);

    image8u_from_image32f(buffer_ptr, in_ptr);

    if (method != METHOD_GRADE) {
        destroy_image32f(in_ptr);
    }

    init_image8u(grayscale_ptr, image->bitmap.width, image->bitmap.height, 32);

    switch (method) {
        case METHOD_ERODE:
            erode(&cells, image, grayscale_ptr, buffer_ptr);
            break;
        case METHOD_PEEKPOINTS:
            peekpoints(&cells, buffer_ptr);
            break;
        case METHOD_GRADE:
            grade(&cells, in_ptr);
            destroy_image32f(in_ptr);
            break;
        default:
            printf("Warning: No method selected (Use -m or --method)\n");
            break;
    }

    destroy_image8u(&grayscale);
    destroy_image8u(&buffer);

    uint32_t count = 0;

    for (uint32_t i = 0; i < cells.len; i++) {
        uint32_t x = cells.data[i].x;
        uint32_t y = cells.data[i].y;

        count += draw_cross(&image->bitmap, x, y, 255, 0, 0, method == METHOD_GRADE ? 0 : 70);
    }

    printf("Found %u cells\n", count);
    
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

void create_kernel() {
    // Initialize kernel
    Kernel kernel;
    
    switch (kernel_type) {
        case KERNEL_TYPE_GAUSSIAN:
            printf("Initializing gaussian kernel with size: %d sigma: %f\n", kernel_size, kernel_arg);
            init_gaussian_kernel(&kernel, kernel_size, kernel_arg);
            break;
        case KERNEL_TYPE_LAPLACIAN:
            printf("Initializing laplacian kernel with size: %d and scale: %f\n", kernel_size, kernel_arg);
            init_laplacian_kernel(&kernel, kernel_size, kernel_arg);
            break;
        case KERNEL_TYPE_LOG:
            printf("Initializing LoG with size: %d sigma: %f and scale %f\n", kernel_size, kernel_arg, kernel_arg2);
            init_log_kernel(&kernel, kernel_size, kernel_arg, kernel_arg2);
            break;
        case KERNEL_TYPE_DOG:
            printf("Initializing DoG with size: %d sigma1: %f and sigma2: %f\n", kernel_size, kernel_arg, kernel_arg2);
            init_dog_kernel(&kernel, kernel_size, kernel_arg, kernel_arg2);
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

        if (mode == OPT_METHOD) {
            method = atoi(argv[i]);
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

        if (strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--method") == 0) {
            mode = OPT_METHOD;
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
        printf("  -b --kernel-arg2\t Set kernel argument 2\n");
        printf("  -m --method\t Set method\n");
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
        printf("  4\t DoG\n");

        // Print method types
        printf("Method types:\n");
        printf("  0\t NONE\n");
        printf("  1\t ERODE\n");
        printf("  2\t PEEKPOINTS\n");
        printf("  3\t GRADE\n");

        return 0;
    }

    printf("Running with %d kernels\n", kernels.len);

    // if not pass_dir exists, create it
    struct stat st;
    
    if (!S_ISDIR(stat(pass_dir, &st) == 0 ? st.st_mode : 0)) {
        mkdir(pass_dir, 0777);
    }

    // If kernels are empty set defauls based on method
    if (kernels.len == 0) {
        switch (method) {
            case METHOD_ERODE:
                kernel_type = KERNEL_TYPE_GAUSSIAN;
                kernel_size = 14;
                kernel_arg = 3.3f;
                create_kernel();

                kernel_type = KERNEL_TYPE_LOG;
                kernel_size = 9;
                kernel_arg = 0.9f;
                kernel_arg2 = 100.0f;
                create_kernel();
                break;
            case METHOD_PEEKPOINTS:
                kernel_type = KERNEL_TYPE_GAUSSIAN;
                kernel_size = 11;
                kernel_arg = 3.3f;
                create_kernel();

                kernel_type = KERNEL_TYPE_LAPLACIAN;
                kernel_size = 21;
                kernel_arg = 1.0f;
                kernel_arg2 = 1.0f;
                create_kernel();
                break;
            case METHOD_GRADE:
                kernel_type = KERNEL_TYPE_GAUSSIAN;
                kernel_size = 9;
                kernel_arg = 3.0f;
                create_kernel();
            default:
                break;
        }
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
