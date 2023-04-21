#ifndef GLFRAMEBUFFER_H
#define GLFRAMEBUFFER_H

#include "gl_texture.h"

class GLFrameBufferBase {
public:
    virtual void bind() const = 0;
    virtual ~GLFrameBufferBase() { }
};

class GLFrameBuffer : public GLFrameBufferBase {
public:
    void create(glm::vec2 size);
    void bind() const override;

private:
    unsigned int m_fbo {};
    GLTexture m_texture {};
};

#endif // GLFRAMEBUFFER_H
