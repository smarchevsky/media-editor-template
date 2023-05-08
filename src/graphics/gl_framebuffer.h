#ifndef GLFRAMEBUFFER_H
#define GLFRAMEBUFFER_H

#include "gl_texture.h"
#include <glm/vec4.hpp>
#include <memory>

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
    void clear(bool withDepth = true);
    void setClearColor(const glm::vec4& color) { m_clearColor = color; }

    virtual ~GLFrameBufferBase() { }
};

class GLFrameBuffer : public GLFrameBufferBase {
public:
    ~GLFrameBuffer();

    void create(glm::vec2 size,
        GLTexture2D::Format format, bool withDepthBuffer = false);

    void reset();
    void bind() const override;
    bool hasDepth() const override { return !!m_depthTexture; }
    void generateMipMap();

    auto& getTexture() { return m_colorTexture; }

private:
    unsigned int m_fbo = 0;
    std::shared_ptr<GLTexture2D> m_colorTexture;
    std::shared_ptr<GLDepthBuffer2D> m_depthTexture;
};

#endif // GLFRAMEBUFFER_H
