#include "gl_framebuffer.h"

#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>

#include <iostream>

size_t GLFrameBufferBase::s_currentBuffer = 0;

void GLFrameBufferBase::staticUnbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }
void GLFrameBufferBase::staticSetViewport(int x, int y, int width, int height) { glViewport(x, y, width, height); }

void GLFrameBufferBase::staticEnableDepthTest(bool enable)
{
    if (enable)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);
}

void GLFrameBufferBase::enableDepthTest(bool enabled)
{
    bind();
    m_depthEnabled = enabled;
    staticEnableDepthTest(m_depthEnabled);
    if (m_depthEnabled) {
        glClear(GL_DEPTH_BUFFER_BIT);
    }
}

void GLFrameBufferBase::clear(float r, float g, float b, float a)
{
    bind();
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | (m_depthEnabled ? GL_DEPTH_BUFFER_BIT : 0));
}

void GLFrameBuffer::create(glm::vec2 size)
{
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    m_colorTexture = std::make_shared<GLTexture2D>(size);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTexture->getHandle(), 0);

    m_depthTexture = std::make_shared<GLDepthBuffer2D>(size);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthTexture->getHandle());

    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE
        && "ERROR::FRAMEBUFFER:: Framebuffer is not complete!");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLFrameBuffer::~GLFrameBuffer() { reset(); }

void GLFrameBuffer::reset()
{
    if (m_fbo) {
        glDeleteFramebuffers(1, &m_fbo);
        m_fbo = 0;
    }
    m_colorTexture.reset();
}

void GLFrameBuffer::bind() const
{
    if (s_currentBuffer != m_fbo) {
        s_currentBuffer = m_fbo;

        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        staticEnableDepthTest(m_depthEnabled);

        if (m_colorTexture) {
            auto size = m_colorTexture->getSize();
            glViewport(0, 0, size.x, size.y);
        }
    }
}

void GLFrameBuffer::generateMipMap()
{
    if (m_colorTexture) {
        m_colorTexture->generateMipMap();
    }
}
