#ifndef GLTEXTURE_H
#define GLTEXTURE_H
#include "helper_general.h"
#include "image.h"

class GLTexture : NoCopy<GLTexture> {
public:
    // clang-format off
    enum class Format : uint8_t { RGB, RGBA };
    enum class OutOfBoundsBehavior { Repeat, MirrorRepeat, ClampEdge, ClampBorder };
    // clang-format on

public:
    GLTexture() = default;
    GLTexture(const Image& img) { fromImage(img); }
    ~GLTexture();
    bool fromImage(const Image& img);
    auto getHandle() const { return m_textureHandle; }

    bool setOutOfBoundsBehavior(OutOfBoundsBehavior);

protected:
    unsigned int m_textureHandle {};
    Format m_internalFormat = Format::RGBA;
    void bind();
};

#endif // GLTEXTURE_H
