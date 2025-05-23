#include "util_texture.h"

TextureData texture_load_data(str_t path) {
    StackMemoryArena scratch_arena = arena_get_scratch(0);

    TextureData texture_data;
    int nr_channels;
    texture_data.data = stbi_load(str_to_cstr(scratch_arena.arena, path), &texture_data.width, &texture_data.height, &nr_channels, 0);

    arena_release_scratch(&scratch_arena);

    return texture_data;
}

void texture_free(TextureData texture_data) {
    stbi_image_free(texture_data.data);
}