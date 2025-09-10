#pragma once

#include <stdint.h>

#include "UtilWindow/src/util_opengl_bindings.h"

#include "util_texture.h"

typedef struct FrameBuffer {
    uint32_t id;
    Texture texture;
} FrameBuffer;

FrameBuffer framebuffer_create(uint32_t width, uint32_t height);
void framebuffer_destroy(FrameBuffer framebuffer);