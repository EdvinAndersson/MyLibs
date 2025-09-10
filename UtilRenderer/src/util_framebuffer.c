#include "util_framebuffer.h"

FrameBuffer framebuffer_create(uint32_t width, uint32_t height) {
    FrameBuffer framebuffer = { 0 };

    glGenFramebuffers(1, &framebuffer.id);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.id);

    framebuffer.texture = texture_2d_create(width, height, TextureFormat_RGB, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer.texture.handle, 0);

    uint32_t rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)

    UTIL_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer failed to create...");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return framebuffer;
}

void framebuffer_destroy(FrameBuffer framebuffer) {
    glDeleteFramebuffers(1, &framebuffer.id);
}