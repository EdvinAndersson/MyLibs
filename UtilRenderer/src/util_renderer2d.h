#pragma once

#include <stdint.h>

#include "Util/src/util_math.h"
#include "Util/src/util_memory_arena.h"

#include "UtilWindow/src/util_opengl_bindings.h"

#include "util_shader.h"
#include "util_texture.h"
#include "util_shaders.h"

void r2d_init(MemoryArena *arena);
void r2d_clear(vec4_t color);
void r2d_update_projection(vec2_t window_size);

void r2d_render_rect(vec2_t position, vec2_t size, vec4_t color, float rotation, vec2_t pivot);

void r2d_render_sprite(vec2_t position, vec2_t size, Texture texture, vec4_t color, float rotation, vec2_t pivot);

void r2d_flush();
void _r2d_begin_batch();
void _r2d_end_batch();

void _r2d_create_texture_3d();

Texture r2d_create_texture_from_file(str_t path, uint8_t bilinear);
Texture r2d_create_texture_from_data(TextureData texture_data, uint8_t bilinear);
Texture r2d_create_texture(unsigned char *data, uint32_t width, uint32_t height, uint8_t bilinear);