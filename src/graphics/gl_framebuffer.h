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

public:
    virtual void bind() const = 0;

    virtual ~GLFrameBufferBase() { }
    void clear(float, float, float, float);
};

class GLFrameBuffer : public GLFrameBufferBase {
public:
    void create(glm::vec2 size);

    void bind() const override;
    void generateMipMap();

    auto& getTexture() { return m_textureInstance; }

private:
    unsigned int m_fbo {};
    std::shared_ptr<GLTexture> m_textureInstance {};
};

#endif // GLFRAMEBUFFER_H
