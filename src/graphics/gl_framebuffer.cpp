#include "gl_framebuffer.h"

#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>

void GLFrameBuffer::create(glm::vec2 size)
{
    glGenFramebuffers(1, &m_fbo);
    bind();

    m_texture.createEmpty(size);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture.getHandle(), 0);
}

void GLFrameBuffer::bind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
}
