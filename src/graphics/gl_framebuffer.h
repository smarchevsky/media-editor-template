#ifndef GLFRAMEBUFFER_H
#define GLFRAMEBUFFER_H

#include "gl_texture.h"
#include <memory>

class GLFrameBufferBase {
protected:
    static size_t s_currentBuffer;
    bool m_depthEnabled = false;

public:
    static void staticUnbind();
    static void staticSetViewport(int x, int y, int width, int height);
    static void staticEnableDepthTest(bool enable);
    void enableDepthTest(bool enabled);

public:
    virtual void bind() const = 0;
    void clear(float, float, float, float = 1.f);

    virtual ~GLFrameBufferBase() { }
};

class GLFrameBuffer : public GLFrameBufferBase {
public:
    ~GLFrameBuffer();
    void create(glm::vec2 size);
    void reset();

    void bind() const override;
    void generateMipMap();

    auto& getTexture() { return m_colorTexture; }

private:
    unsigned int m_fbo = 0;
    std::shared_ptr<GLTexture2D> m_colorTexture;
    std::shared_ptr<GLDepthBuffer2D> m_depthTexture;
};

#endif // GLFRAMEBUFFER_H
