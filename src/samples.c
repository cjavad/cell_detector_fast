#include "samples.h"
#include "bitmap.h"
#include <sys/stat.h>

const char* SAMPLE_TYPES[] = {
    "easy",
    "medium",
    "hard",
    "impossible"
};


void resolve_sample_path(char* output, uint8_t sample_type, char* sample_name) {
    sprintf(output, "../../samples/%s", SAMPLE_TYPES[sample_type]);

    if (sample_name) {
        sprintf(output + strlen(output), "/%s.bmp", sample_name);
    }
}

void resolve_output_path(char* output, uint8_t sample_type, char* sample_name) {
    sprintf(output, "./res/%s", SAMPLE_TYPES[sample_type]);

    // if directory does not exist, create it
    if (mkdir(output, 0777) == -1) {
        // Directory already exists
    }

    if (sample_name) {
        sprintf(output + strlen(output), "/%s.bmp", sample_name);
    }
}

void get_samples(sample_t*** samples, uint32_t* count, uint8_t sample_type) {
    char* path = malloc(512);

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

        (*samples)[i]->sample_path = malloc(strlen(path) + len + 1);
        sprintf((*samples)[i]->sample_path, "%s/%s", path, name);

        init_bitmap((*samples)[i]->output_bmp);
        i++;
    }

    closedir(d);
}

void write_sample(sample_t *sample) {
    char* path = malloc(512);
    
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