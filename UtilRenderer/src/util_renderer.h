#pragma once

#include <stdint.h>

typedef struct render_batch {
    void *vertices, *vertices_begin;
    uint32_t count;
    uint32_t vertex_array, vertex_buffer;
} render_batch_t;