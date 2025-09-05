#pragma once

#include <stdint.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "Util/src/util_math.h"
#include "Util/src/util_memory_arena.h"

#include "UtilWindow/src/util_opengl_bindings.h"

#include "util_shader.h"
#include "util_texture.h"
#include "util_shaders.h"
#include "util_key_map.h"

#define FONT_SIZE 64

typedef struct render_batch {
    void *vertices, *vertices_begin;
    uint32_t count;
    uint32_t vertex_array, vertex_buffer;
} render_batch_t;

void r2d_init(MemoryArena *arena);
void r2d_clear(vec4_t color);
void r2d_update_projection(vec2_t window_size);

void r2d_render_rect(vec2_t position, vec2_t size, vec4_t color, float rotation, vec2_t pivot);
void r2d_render_sprite(vec2_t position, vec2_t size, Texture texture, vec4_t color, float rotation, vec2_t pivot);
void r2d_render_rect_rounded(vec2_t position, vec2_t size, vec4_t color, float rotation, vec2_t pivot, float rounded_radius);
void r2d_render_sprite_rounded(vec2_t position, vec2_t size, Texture texture, vec4_t color, float rotation, vec2_t pivot, float rounded_radius);
void r2d_render_thick_line(vec2_t start, vec2_t end, float thickness, vec4_t color);

void r2d_render_text(str_t text, vec2_t position, float scale, vec3_t color);
cmap_map_t *r2d_get_character_map();
uint32_t r2d_get_line_height();

Texture r2d_texture_array_add(str_t path, uint8_t bilinear);

void r2d_batch_flush(render_batch_t *batch, uint32_t indices_per, Shader shader, TextureArray texture_array);
void _r2d_batch_begin(render_batch_t *batch);
void _r2d_batch_end(render_batch_t *batch);

void r2d_flush();