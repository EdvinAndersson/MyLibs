#pragma once

#include <stdint.h>

#include "Util/src/util_math.h"
#include "Util/src/util_string.h"

typedef struct cmap_char {
    Texture texture;
    vec2_t size;
    vec2_t bearing;
    uint32_t advance;
} cmap_char_t;

#define MAX_SIZE 256

typedef struct cmap_map {
    uint32_t size;
    str_t keys[MAX_SIZE];
    cmap_char_t values[MAX_SIZE];
} cmap_map_t;

inline int32_t cmap_get_index(cmap_map_t *map, str_t key) {
    for (uint32_t i = 0; i < map->size; i++) {
        if (str_equal(map->keys[i], key)) {
            return i;
        }
    }
    return -1;
}

inline void cmap_insert(MemoryArena *arena, cmap_map_t *map, str_t key, cmap_char_t value) {
    int32_t index = cmap_get_index(map, key);

    if (index == -1) {
        map->keys[map->size] = str_copy(arena, key);
        map->values[map->size] = value;
        map->size++;
    } else {
        map->values[index] = value;
    }
}