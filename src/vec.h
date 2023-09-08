#pragma once

#include <stdint.h>

#define Vec(T) struct { \
    uint32_t len; \
    uint32_t cap; \
    T* data; \
}

#define vec_init(vec) { \
    (vec)->len = 0; \
    (vec)->cap = 0; \
}

#define vec_grow(vec) { \
    if (!(vec)->cap) { \
        (vec)->cap = 16; \
        (vec)->data = malloc(sizeof(*(vec)->data) * (vec)->cap); \
    } else { \
        uint32_t new_cap = (vec)->cap * 2; \
        (vec)->data = realloc((vec)->data, sizeof(*(vec)->data) * new_cap); \
        (vec)->cap = new_cap; \
    } \
}

#define vec_push(vec, item) { \
    if ((vec)->cap <= (vec)->len) { \
        vec_grow(vec); \
    } \
     \
    (vec)->data[(vec)->len++] = item; \
}

#define vec_pop(vec) (vec)->data[--(vec)->len]

#define vec_free(vec) free((vec)->data)
