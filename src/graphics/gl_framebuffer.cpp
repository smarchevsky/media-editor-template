#include "gl_framebuffer.h"

#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>

#include <iostream>

size_t GLFrameBufferBase::m_currentBuffer = 0;

void GLFrameBuffer::create(glm::vec2 size)
{
    glGenFramebuffers(1, &m_fbo);
    bind();

    m_textureInstance = SHARED_TEXTURE();
    m_textureInstance->createEmpty(size);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
        m_textureInstance->getHandle(), 0);
}

GLFrameBuffer::~GLFrameBuffer() { reset(); }

void GLFrameBuffer::reset()
{
    if (m_fbo) {
        glDeleteFramebuffers(1, &m_fbo);
        m_fbo = 0;
    }
    m_textureInstance.reset();
}

void GLFrameBuffer::bind() const
{
    if (m_currentBuffer != m_fbo) {
        m_currentBuffer = m_fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

        if (m_textureInstance) {
            auto size = m_textureInstance->getSize();
            glViewport(0, 0, size.x, size.y);
        }
    }
}

void GLFrameBuffer::generateMipMap()
{
    if (m_textureInstance) {
        m_textureInstance->generateMipMap();
    }
}

void GLFrameBufferBase::unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }
void GLFrameBufferBase::setViewport(int x, int y, int width, int height) { glViewport(x, y, width, height); }

void GLFrameBufferBase::enableDepthTest()
{
    glEnable(GL_DEPTH_TEST);
}

void GLFrameBufferBase::clear(float r, float g, float b, float a)
{
    bind();

    // glViewport(0, 0, width, height);
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
