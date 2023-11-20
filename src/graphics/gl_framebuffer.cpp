#include "gl_framebuffer.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

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

void GLFrameBuffer::createInternal(glm::vec2 size, GLTexture2D::Format format)
{
    if (!m_fbo)
        glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    m_colorTexture = std::make_shared<GLTexture2D>(size, format);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTexture->getHandle(), 0);
}

void GLFrameBuffer::create(glm::vec2 size, GLTexture2D::Format format)
{
    createInternal(size, format);

    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE && "ERROR::FRAMEBUFFER:: Framebuffer is not complete!");
}

void GLFrameBuffer::resize(glm::vec2 newSize)
{
    if (m_colorTexture)
        m_colorTexture->createFromRawData(newSize, m_colorTexture->m_format, nullptr);
}

GLFrameBuffer::~GLFrameBuffer()
{
    if (m_fbo) {
        glDeleteFramebuffers(1, &m_fbo);
        m_fbo = 0;
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

            m_colorTexture->m_mipmapDirty = true; // when texture will be attached to shader - generate mipmap if not generated
        }
    }
}

void GLFrameBuffer::writeFramebufferToFile(const std::filesystem::__cxx11::path& path)
{
    if (m_colorTexture) {
        auto size = m_colorTexture->getSize();
        uint8_t* data = new uint8_t[size.x * size.y * 3];
        glBindTexture(GL_TEXTURE_2D, m_colorTexture->getHandle());
        glReadPixels(0, 0, size.x, size.y, GL_RGB, GL_UNSIGNED_BYTE, data);
        stbi_write_png(path.c_str(), size.x, size.y, 3, data, size.x * 3);
    }
}

void GLFrameBufferDepth::create(glm::vec2 size, GLTexture2D::Format format)
{
    createInternal(size, format);
    m_depthTexture = std::make_shared<GLDepthBuffer2D>(size);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthTexture->getHandle());

    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE && "ERROR::FRAMEBUFFER:: Framebuffer is not complete!");
}

void GLFrameBufferDepth::resize(glm::vec2 newSize)
{
    if (m_depthTexture)
        m_depthTexture->create(newSize);
}
