#include "gl_framebuffer.h"

#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>

#include <iostream>

size_t GLFrameBufferBase::s_currentBuffer = 0;

void GLFrameBufferBase::staticUnbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }
void GLFrameBufferBase::staticSetViewport(int x, int y, int width, int height) { glViewport(x, y, width, height); }

void GLFrameBufferBase::clear(bool withDepth)
{
    bind();
    glClearColor(m_clearColor.r, m_clearColor.g, m_clearColor.b, m_clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | ((hasDepth() && withDepth) ? GL_DEPTH_BUFFER_BIT : 0));
    // if(!hasDepth() && withDepth){
    //     LOGE("Trying to clear depth of framebuffer without depth. Probably it must have depth?");
    // }
}

void GLFrameBuffer::create(glm::vec2 size, GLTexture2D::Format format, bool withDepthBuffer)
{
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    m_colorTexture = std::make_shared<GLTexture2D>(size, format);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTexture->getHandle(), 0);

    if (withDepthBuffer) {
        m_depthTexture = std::make_shared<GLDepthBuffer2D>(size);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthTexture->getHandle());

        assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE
            && "ERROR::FRAMEBUFFER:: Framebuffer is not complete!");
    }

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
