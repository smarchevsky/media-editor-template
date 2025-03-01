#ifndef GLFRAMEBUFFER_H
#define GLFRAMEBUFFER_H

#include "gl_texture.h"
#include <glm/vec4.hpp>
#include <memory>
#include <vector>

class GLFrameBufferBase {
protected:
    static size_t s_currentBuffer;
    glm::vec4 m_clearColor = glm::vec4(0, 0, 0, 1);

public:
    static void staticUnbind();
    static void staticSetViewport(int x, int y, int width, int height);
    static void staticEnableDepthTest(bool enable);

public:
    virtual void bind() const = 0;
    virtual bool hasDepth() const = 0;
    virtual Image getBufferImage(TexelFormat format) = 0;
    void clear();

    void setClearColor(const glm::vec4& color) { m_clearColor = color; }

    virtual ~GLFrameBufferBase() { }
};

class GLFrameBuffer : public GLFrameBufferBase {
protected:
    unsigned int m_fbo = 0;
    std::shared_ptr<GLTexture2D> m_colorTexture;

public:
    GLFrameBuffer() = default;
    GLFrameBuffer& operator=(GLFrameBuffer&& rhs)
    {
        m_fbo = rhs.m_fbo, rhs.m_fbo = 0;
        m_clearColor = rhs.m_clearColor;
        m_colorTexture = std::move(rhs.m_colorTexture);
        return *this;
    }
    GLFrameBuffer(glm::vec2 size, TexelFormat format) { create(size, format); }
    GLFrameBuffer(GLFrameBuffer&& rhs) { *this = std::move(rhs); }
    ~GLFrameBuffer();

    void bind() const override;
    bool hasDepth() const override { return false; }

    virtual void create(glm::vec2 size, TexelFormat format);
    virtual void resize(glm::vec2 newSize);

    // update framebuffer's texture from Image, must match size
    bool setImage(const Image& image);

    const auto& getTexture() const { return m_colorTexture; }

    Image getBufferImage(TexelFormat format) override;
};

class GLFrameBufferDepth : public GLFrameBuffer {
    std::shared_ptr<GLDepthBuffer2D> m_depthTexture;

public:
    bool hasDepth() const override { return true; }
    void create(glm::vec2 size, TexelFormat format) override;
    void resize(glm::vec2 newSize) override;
};

#endif // GLFRAMEBUFFER_H
