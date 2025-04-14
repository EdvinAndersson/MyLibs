#pragma once

#include <stdint.h>

#include "vendor/stb_image/stb_image.h"

#include "Util/src/util_string.h"
#include "Util/src/util_memory_arena.h"

typedef struct Texture {
    uint32_t handle;
    float use_bilinear;
    uint32_t width, height;
} Texture;

typedef struct TextureData {
    unsigned char *data;
    uint32_t width, height;
} TextureData;

TextureData texture_load_data(MemoryArena *arena, str_t path);
void texture_free(TextureData texture_data);