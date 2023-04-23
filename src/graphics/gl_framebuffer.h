#ifndef GLFRAMEBUFFER_H
#define GLFRAMEBUFFER_H

#include "gl_texture.h"

class GLFrameBufferBase {
protected:
    static size_t m_currentBuffer;

public:
    virtual void bind() const = 0;
    virtual ~GLFrameBufferBase() { }
};

class GLFrameBuffer : public GLFrameBufferBase {
public:
    void create(glm::vec2 size);
    void bind() const override;
    auto& getTexture() { return m_texture; }

private:
    unsigned int m_fbo {};
    GLTexture m_texture {};
};

#endif // GLFRAMEBUFFER_H
