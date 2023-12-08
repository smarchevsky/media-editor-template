#include "gl_framebuffer.h"

#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>

#include <iostream>

size_t GLFrameBufferBase::s_currentBuffer = -1;

void GLFrameBufferBase::staticUnbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }
void GLFrameBufferBase::staticSetViewport(int x, int y, int width, int height) { glViewport(x, y, width, height); }

void GLFrameBufferBase::clear()
{
    bind();
    glClearColor(m_clearColor.r, m_clearColor.g, m_clearColor.b, m_clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | ((hasDepth()) ? GL_DEPTH_BUFFER_BIT : 0));
}

void GLFrameBuffer::create(glm::vec2 size, TexelFormat format)
{
    if (!m_fbo)
        glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    m_colorTexture = std::make_shared<GLTexture2D>(size, format);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTexture->getHandle(), 0);

    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE && "ERROR::FRAMEBUFFER:: Framebuffer is not complete!");
}

void GLFrameBuffer::resize(glm::vec2 newSize)
{
    if (m_colorTexture)
        m_colorTexture->createFromRawData(newSize, m_colorTexture->getFormat(), nullptr);
}

GLFrameBuffer::~GLFrameBuffer()
{
    if (m_fbo) {
        glDeleteFramebuffers(1, &m_fbo);
    }
}

void GLFrameBuffer::bind() const
{
    if (s_currentBuffer != m_fbo) {
        s_currentBuffer = m_fbo;

        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

        if (m_colorTexture) {
            auto size = m_colorTexture->getSize();
            glViewport(0, 0, size.x, size.y);

            // this framebuffer is going to be modified, so
            // when texture will be attached to shader - mipmap will be generated
            m_colorTexture->m_mipmapDirty = true;
        }
    }
}

// void GLFrameBuffer::getRawPixelData(std::vector<uint8_t>& byteArray, TexelFormat format)
// {
//     if (m_colorTexture) {
//         GLTexture2D::getRawData(byteArray, m_colorTexture->getHandle(), m_colorTexture->getSize(), format);
//     }
// }

Image GLFrameBuffer::getBufferImage(TexelFormat format)
{
    Image image;
    if (m_colorTexture) {
        image = m_colorTexture->toImage(format);
    }
    return image;
}

void GLFrameBufferDepth::create(glm::vec2 size, TexelFormat format)
{
    GLFrameBuffer::create(size, format);

    m_depthTexture = std::make_shared<GLDepthBuffer2D>(size);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthTexture->getHandle());

    // check framebuffer completion again
    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE && "ERROR::RENDERBUFFER:: Renderbuffer is not complete!");
}

void GLFrameBufferDepth::resize(glm::vec2 newSize)
{
    GLFrameBuffer::resize(newSize);
    if (m_depthTexture)
        m_depthTexture->create(newSize);
}
