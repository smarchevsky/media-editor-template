#ifndef GLFRAMEBUFFER_H
#define GLFRAMEBUFFER_H

#include "gl_texture.h"
#include <memory>

class GLFrameBufferBase {
protected:
    static size_t m_currentBuffer;

public:
    static void unbind();
    static void setViewport(int x, int y, int width, int height);
    static void enableDepthTest();

public:
    virtual void bind() const = 0;
    void clear(float, float, float, float = 1.f);
    //virtual void enableDepth() = 0;

    virtual ~GLFrameBufferBase() { }
};

class GLFrameBuffer : public GLFrameBufferBase {
public:
    ~GLFrameBuffer();
    void create(glm::vec2 size);
    void reset();

    void bind() const override;
    void generateMipMap();

    auto& getTexture() { return m_textureInstance; }

private:
    unsigned int m_fbo = 0;
    std::shared_ptr<GLTexture> m_textureInstance;
};

#endif // GLFRAMEBUFFER_H
