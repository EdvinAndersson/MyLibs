#pragma once

#include <stdint.h>

#include "Util/src/util_math.h"
#include "Util/src/util_string.h"

#include "UtilWindow/src/util_opengl_bindings.h"

typedef struct Shader {
    uint32_t id;
} Shader;

void shader_use(Shader shader);
void shader_set_v4(Shader shader, char *name, vec4_t v);
void shader_set_v3(Shader shader, char *name, vec3_t v);
void shader_set_v2(Shader shader, char *name, vec2_t v);
void shader_set_mat4(Shader shader, char *name, mat4_t *v);
void shader_set_float(Shader shader, char *name, float v);
void shader_set_int(Shader shader, char *name, int v);

Shader shader_create(MemoryArena *arena, str_t vertex_shader_string, str_t fragment_shader_string);
