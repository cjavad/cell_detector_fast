#include "samples.h"
#include "bitmap.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
    #include <direct.h>
    #define mkdir(path, mod) _mkdir(path)
#else 
    #include <sys/stat.h>
#endif

const char* SAMPLE_TYPES[] = {
    "easy",
    "medium",
    "hard",
    "impossible"
};

char SAMPLE_PATH[512] = "../../samples";
char RESULT_PATH[512] = "./res";

void resolve_sample_path(char* output, uint8_t sample_type, char* sample_name) {
    sprintf(output, "%s/%s", SAMPLE_PATH, SAMPLE_TYPES[sample_type]);

    if (sample_name) {
        sprintf(output + strlen(output), "/%s.bmp", sample_name);
    }
}

void resolve_output_path(char* output, uint8_t sample_type, char* sample_name) {
    // if directory does not exist, create it
    if (mkdir(RESULT_PATH, 0777) == -1) {
        // Directory already exists
    }

    sprintf(output, "%s/%s", RESULT_PATH, SAMPLE_TYPES[sample_type]);

    // if directory does not exist, create it
    if (mkdir(output, 0777) == -1) {
        // Directory already exists
    }

    if (sample_name) {
        sprintf(output + strlen(output), "/%s.bmp", sample_name);
    }
}

void get_samples(sample_t*** samples, uint32_t* count, uint8_t sample_type) {
    char path[512];

    resolve_sample_path(path, sample_type, NULL);
    
    DIR* d = opendir(path);
    struct dirent* dir;

    if (!d) {
        printf("Could not open directory %s\n", path);
        exit(1);
    }

    uint32_t i = 0;

    // Preallocate the array
    while ((dir = readdir(d)) != NULL) {
        if (dir->d_type != DT_REG) continue;

        char* name = dir->d_name;
        char* ext = name + strlen(name) - 4;

        if (strcmp(ext, ".bmp") != 0) continue;

        i++;
    }

    *count = i;
    *samples = malloc(sizeof(sample_t*) * i);

    rewinddir(d);

    i = 0;

    while ((dir = readdir(d)) != NULL) {
        if (dir->d_type != DT_REG) continue;

        char* name = dir->d_name;
        uint32_t len = strlen(name);
        char* ext = name + len - 4;

        if (strcmp(ext, ".bmp") != 0) continue;

        (*samples)[i] = malloc(sizeof(sample_t));
        (*samples)[i]->sample_type = sample_type;

        // Allocate for sample name without extension
        (*samples)[i]->sample_name = malloc(len - 4);
        memcpy((*samples)[i]->sample_name, name, len - 4);
        (*samples)[i]->sample_name[len - 4] = '\0';

        (*samples)[i]->sample_path = malloc(strlen(path) + len + 2);
        sprintf((*samples)[i]->sample_path, "%s/%s", path, name);

        // (*samples)[i]->output_bmp = malloc(sizeof(BitmapImage));

        i++;
    }

    closedir(d);
}

void write_sample(sample_t *sample) {
    char path[512];
    
    resolve_output_path(path, sample->sample_type, sample->sample_name);

    FILE* fp = fopen(path, "wb");
    write_bitmap(fp, sample->output_bmp);

    fclose(fp);
}

void write_samples(sample_t **samples, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        write_sample(samples[i]);
    }
}

void free_sample(sample_t *sample) {
    free(sample->sample_name);
    free(sample->sample_path);
    free_bitmap(sample->output_bmp);
    free(sample);
}