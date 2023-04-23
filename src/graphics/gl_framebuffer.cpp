#include "gl_framebuffer.h"

#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>

#include <iostream>

size_t GLFrameBufferBase::m_currentBuffer = 0;

void GLFrameBuffer::create(glm::vec2 size)
{
    glGenFramebuffers(1, &m_fbo);
    bind();

    m_textureInstance.createEmpty(size);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
        m_textureInstance.get()->getHandle(), 0);
}

void GLFrameBuffer::bind() const
{
    if (m_currentBuffer != m_fbo) {
        m_currentBuffer = m_fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

        if (m_textureInstance.get()) {
            auto size = m_textureInstance.get()->getSize();
            glViewport(0, 0, size.x, size.y);
        }

        // LOGE("FBO: " << m_fbo << " binded");
    }
}

void GLFrameBufferBase::clear(float r, float g, float b, float a)
{
    bind();

    // glViewport(0, 0, width, height);
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT);
}
