#ifndef GLFRAMEBUFFER_H
#define GLFRAMEBUFFER_H

#include "gl_texture.h"
#include <memory>

class GLFrameBufferBase {
protected:
    static size_t m_currentBuffer;

public:
    virtual void bind() const = 0;
    virtual ~GLFrameBufferBase() { }
    void clear(float, float, float, float);
};

class GLFrameBuffer : public GLFrameBufferBase {
public:
    void create(glm::vec2 size);
    void bind() const override;
    auto& getTexture() { return m_textureInstance; }

private:
    unsigned int m_fbo {};
    GLTextureInstance m_textureInstance {};
};

#endif // GLFRAMEBUFFER_H
