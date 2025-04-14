#include "util_texture.h"

TextureData texture_load_data(MemoryArena *arena, str_t path) {
    TextureData texture_data;
    int nr_channels;
    texture_data.data = stbi_load(str_to_cstr(arena, path), &texture_data.width, &texture_data.height, &nr_channels, 0);

    return texture_data;
}

void texture_free(TextureData texture_data) {
    stbi_image_free(texture_data.data);
}