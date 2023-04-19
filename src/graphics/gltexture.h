#ifndef GLTEXTURE_H
#define GLTEXTURE_H
#include "helper_general.h"
#include "image.h"

class GLTexture : NoCopy<GLTexture> {
public:
    enum class Format : uint8_t {
        RGB,
        RGBA
    };

public:
    GLTexture() = default;
    GLTexture(const Image& img) { fromImage(img); }
    ~GLTexture();
    bool fromImage(const Image& img);

    auto getHandle() const { return m_textureHandle; }

protected:
    unsigned int m_textureHandle {};
    Format m_internalFormat = Format::RGBA;
};

#endif // GLTEXTURE_H
