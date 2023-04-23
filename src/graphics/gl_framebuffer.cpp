#include "gl_framebuffer.h"

#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>

#include <iostream>

size_t GLFrameBufferBase::m_currentBuffer = 0;

void GLFrameBuffer::create(glm::vec2 size)
{
    glGenFramebuffers(1, &m_fbo);
    bind();

    m_texture.createEmpty(size);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture.getHandle(), 0);
}

void GLFrameBuffer::bind() const
{
    if (m_currentBuffer != m_fbo) {
        m_currentBuffer = m_fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

        //LOGE("FBO: " << m_fbo << " binded");
    }
}
