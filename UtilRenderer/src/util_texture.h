#pragma once

#include <stdint.h>

#include "vendor/stb_image/stb_image.h"

#include "Util/src/util_string.h"
#include "Util/src/util_memory_arena.h"

#include "UtilWindow/src/util_opengl_bindings.h"

typedef enum TextureFormat {
    TextureFormat_RED,
    TextureFormat_RGB,
    TextureFormat_RGBA
} TextureFormat;

typedef struct TextureArray {
    uint32_t handle;
    uint32_t next_slot;
    TextureFormat texture_format;
    uint32_t slot_size;
} TextureArray;

typedef struct Texture {
    uint32_t handle;
    float use_bilinear;
    uint32_t format;
    uint32_t width, height;
} Texture;

typedef struct TextureData {
    unsigned char *data;
    uint32_t width, height;
    uint32_t channels;
} TextureData;

Texture texture_2d_create(uint32_t width, uint32_t height, TextureFormat texture_format, void *pixel_data);

TextureArray texture_3d_create(uint32_t texture_size, uint32_t texture_count, TextureFormat texture_format);
Texture texture_3d_add(TextureArray *texture_array, uint32_t width, uint32_t height, uint8_t bilinear, TextureFormat data_format, void *data);

TextureData texture_load_data(str_t path);
void texture_free(TextureData texture_data);