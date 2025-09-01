#include "util_texture.h"

Texture texture_2d_create(uint32_t width, uint32_t height, TextureFormat texture_format, void *pixel_data) {
    Texture texture = { 0 };

    glGenTextures(1, &texture.handle);
    glBindTexture(GL_TEXTURE_2D, texture.handle);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLint format = GL_RGB;
    switch (texture_format)
    {
        case TextureFormat_RED: {
            format = GL_RED;
        } break;
        case TextureFormat_RGB: {
            format = GL_RGB;
        } break;
        case TextureFormat_RGBA: {
            format = GL_RGBA;
        } break;
    }

    texture.type = TextureType_Texture2D;
    texture.format = format;
    texture.width = width;
    texture.height = height;

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, pixel_data);

    return texture;
}
TextureArray texture_3d_create(uint32_t texture_size, uint32_t texture_count, TextureFormat texture_format) {
    TextureArray texture_array = { 0 };

    glGenTextures(1, &texture_array.handle);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture_array.handle);

    GLint format = GL_RGB;
    switch (texture_format)
    {
        case TextureFormat_RED: {
            format = GL_RED;
        } break;
        case TextureFormat_RGB: {
            format = GL_RGB8;
        } break;
        case TextureFormat_RGBA: {
            format = GL_RGBA8;
        } break;
    }

    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, format, texture_size, texture_size, texture_count);

    texture_array.texture_format = format;
    texture_array.next_slot = 1;
/*
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    uint32_t white = 0xffffffff;

    glBindTexture(GL_TEXTURE_2D_ARRAY, texture_array.handle);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 1, 1, 1, 1, format, GL_UNSIGNED_BYTE, (unsigned char *)&white);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
*/
    return texture_array;
}

Texture texture_3d_add(TextureArray *texture_array, uint32_t width, uint32_t height, uint8_t bilinear, TextureFormat data_format, void *data) {
    Texture texture;

    glBindTexture(GL_TEXTURE_2D_ARRAY, texture_array->handle);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, texture_array->next_slot, width, height, 1, data_format, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    
    texture.handle = texture_array->next_slot;
    texture.format = texture_array->texture_format;
    texture.width = width;
    texture.height = height;
    texture.use_bilinear = bilinear;
    texture.type = TextureType_Texture3D;

    texture_array->next_slot += 1;
    
    return texture;
}

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